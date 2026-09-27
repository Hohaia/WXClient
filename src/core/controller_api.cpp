//
// Created by hohaia on 23/08/2026.
//

#include "controller_api.h"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "helpers.h"
#include "logger.h"

namespace ict
{
    /* PRIVATE FUNCTIONS */
    // Clean up the host address (remove any leading "http://, https://").
    std::string ControllerApi::cleanAddress(const std::string& address)
    {
        std::string s = address;
        for (const auto& prefix : { std::string("https://"), std::string("http://") }) {
            if (s.rfind(prefix, 0) == 0)
            {
                s.erase(0, prefix.size());
                break;
            }
        }
        while (!s.empty() && s.back() == '/')
        {
            s.pop_back();
        }
        return s;
    }

    // XOR each character with the next byte of xorNumber (lowest byte first, repeating), as hex.
    std::string ControllerApi::xorToHex(const std::string& inputString, const std::uint32_t xorNumber)
    {
        std::vector<std::uint8_t> bytes;
        bytes.reserve(inputString.size());
        for (std::size_t i = 0; i < inputString.size(); ++i)
        {
            const auto keyByte = static_cast<std::uint8_t>(xorNumber >> (8 * (i % 4)));
            bytes.push_back(static_cast<std::uint8_t>(static_cast<std::uint8_t>(inputString[i]) ^ keyByte));
        }
        return toHex(bytes);
    }

    // Determine if a controller response is a failure message.
    bool ControllerApi::isFailResponse(const std::string& response)
    {
        const std::string trimmedResponse = trim(response);
        return trimmedResponse.starts_with("FAIL") || trimmedResponse.starts_with("Request Failed");
    }

    // Parse a session random ID from a controller response.
    std::uint32_t ControllerApi::parseSessionRandId(const std::string& response)
    {
        const std::string trimmedResponse = trim(response);
        std::uint32_t value = 0;
        const auto* const end = trimmedResponse.data() + trimmedResponse.size();
        const auto [ptr, ec] = std::from_chars(trimmedResponse.data(), end, value);
        if (ec != std::errc{} || ptr != end)
        {
            throw std::runtime_error("Unexpected session ID response: " + trimmedResponse);
        }
        return value;
    }

    // Extract the name=value pair from a Set-Cookie header value.
    std::string ControllerApi::parseCookiePair(const std::string& setCookieHeader)
    {
        const auto attributes = setCookieHeader.find(';');
        return trim(attributes == std::string::npos ? setCookieHeader : setCookieHeader.substr(0, attributes));
    }

    // Generate a random 32 character hex session id (for controller firmware pre 4.00.1676).
    std::string ControllerApi::generateSessionId()
    {
        std::vector<std::uint8_t> bytes(16);
        if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1)
        {
            throw std::runtime_error("Failed to generate session ID");
        }
        return toHex(bytes);
    }

    // Build "<kind>&Type=<type>&SubType=<subType>&key=value..." with the values encoded.
    std::string ControllerApi::buildParameters(const std::string& kind, const std::string& type
                                             , const std::string& subType, const KeyValueList& params)
    {
        std::string parameters = kind + "&Type=" + type;
        if (!subType.empty())
            parameters += "&SubType=" + subType;
        for (const auto& [key, value] : params)
            parameters += "&" + key + "=" + encodeStringValue(value);
        return parameters;
    }

    // Create the http client.
    httplib::Client ControllerApi::createClient() const
    {
        const std::string cliDomain = std::string(m_isHttps ? "https" : "http") + "://" + m_host + "/";
        httplib::Client cli(cliDomain);
        cli.enable_server_certificate_verification(false);
        cli.set_connection_timeout(5);
        cli.set_read_timeout(5);
        cli.set_write_timeout(5);
        cli.set_keep_alive(true);
        return cli;
    }

    // Build the request requestString depending on WX Controllers firmware version.
    std::string ControllerApi::buildRequestString(const std::string& requestString) const
    {
        if (!m_needsClientSessionId)
        {
            return requestString;
        }
        if (requestString.starts_with("Command&Type=Session&SubType=InitSession")
            || requestString.starts_with("Command&Type=Session&SubType=CheckPassword"))
        {
            return requestString + "&SessionID=" + m_clientSessionId;
        }
        return requestString + "&Sequence=" + std::to_string(m_sequenceNumber);
    }

    // Perform a POST request and read the response as a string.
    std::string ControllerApi::getResponseString(std::string requestString, Reply reply)
    {
        requestString = buildRequestString(requestString);
        if (m_loggedIn && !m_isHttps)
        {
            requestString = encrypt(requestString);
        }
        if (m_loggedIn && m_needsClientSessionId)
        {
            requestString = m_clientSessionId + requestString;
        }
        httplib::Headers headers;
        if (!m_sessionCookie.empty())
        {
            headers.emplace("Cookie", m_sessionCookie);
        }
        auto result = m_client.Post(m_path + requestString, headers);
        if (!result)
        {
            throw std::runtime_error("Could not reach controller: "
                + httplib::to_string(result.error()));
        }
        if (result->status != 200)
        {
            const std::string body = trim(result->body);
            throw std::runtime_error("Controller returned HTTP "
                + std::to_string(result->status) + ": "
                + (body.size() > 200 ? body.substr(0, 200) + "..." : body));
        }
        if (result->has_header("Set-Cookie"))
        {
            m_sessionCookie = parseCookiePair(result->get_header_value("Set-Cookie"));
        }
        std::string response = result->body;
        if (reply == Reply::Decrypt && m_loggedIn && !m_isHttps)
        {
            response = decrypt(response);
        }
        if (m_loggedIn) {m_sequenceNumber += 1;}
        return response;
    }

    // Encrypt a string.
    std::string ControllerApi::encrypt(const std::string& requestString) const
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP cipher context");
        }
        std::array<std::uint8_t, 16> iv{};
        if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to generate Initialising Vector");
        }
        if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, m_aesKey.data(), iv.data()) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        std::vector<std::uint8_t> encryptedBytes(requestString.size() + 16);
        int encryptedLength = 0;
        if (EVP_EncryptUpdate(ctx, encryptedBytes.data(), &encryptedLength
                                , reinterpret_cast<const unsigned char*>(requestString.data())
                                , static_cast<int>(requestString.size())) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to encrypt data");
        }
        int finalLength = 0;
        if (EVP_EncryptFinal_ex(ctx, encryptedBytes.data() + encryptedLength, &finalLength) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }
        EVP_CIPHER_CTX_free(ctx);
        encryptedLength += finalLength;
        const auto encryptedData = std::vector<std::uint8_t>(encryptedBytes.begin(), encryptedBytes.begin() + encryptedLength);
        const std::vector<std::uint8_t> ivBytes(iv.begin(), iv.end());
        return toHex(ivBytes) + toHex(encryptedData);
    }

    // Decrypt a string.
    std::string ControllerApi::decrypt(const std::string& encryptedResponse) const
    {
        const std::string ivStr = encryptedResponse.substr(0, 32);
        const std::string encryptedStr = encryptedResponse.substr(32);
        const auto iv = fromHex(ivStr);
        const auto encryptedBytes = fromHex(encryptedStr);
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP cipher context");
        }
        if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, m_aesKey.data(), iv.data()) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        std::vector<std::uint8_t> decryptedBytes(encryptedBytes.size() + 16);
        int decryptedLength = 0;
        if (EVP_DecryptUpdate(ctx, decryptedBytes.data(), &decryptedLength
                                 , encryptedBytes.data(), static_cast<int>(encryptedBytes.size())) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to decrypt data");
        }
        int finalLength = 0;
        if (EVP_DecryptFinal_ex(ctx, decryptedBytes.data() + decryptedLength, &finalLength) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize decryption");
        }
        EVP_CIPHER_CTX_free(ctx);
        decryptedLength += finalLength;
        return {reinterpret_cast<const char*>(decryptedBytes.data())
                    , static_cast<std::string::size_type>(decryptedLength)};
    }

    // Clear an active session.
    void ControllerApi::clearSession()
    {
        m_loggedIn = false;
        m_sessionCookie.clear();
        m_aesKey = {};
    }

    /* PUBLIC FUNCTIONS */
    // Close any open session before the object goes away.
    ControllerApi::~ControllerApi()
    {
        logout();
    }

    // Return the message from the most recent failed call, for a frontend to display.
    const std::string& ControllerApi::lastError() const
    {
        return m_lastError;
    }

    // Login to the WX controller.
    bool ControllerApi::login(const std::string& userName, const std::string& passwordHash)
    {
        std::string parameters = "Command&Type=Session&SubType=InitSession";
        std::string sessionRandIdString = getResponseString(parameters);
        if (isFailResponse(sessionRandIdString))
        {
            logMessage(LogLevel::Warning, "ControllerApi::login"
                        , "Failed to initialise session: " + sessionRandIdString + ", re-trying with a client generated session ID");
            m_needsClientSessionId = true;
            sessionRandIdString = getResponseString(parameters);
        }
        if (isFailResponse(sessionRandIdString))
        {
            m_lastError = trim(sessionRandIdString);
            logMessage(LogLevel::Error, "ControllerApi::login"
                        , "Failed to initialise session: " + m_lastError);
            return false;
        }
        const std::uint32_t sessionRandIdValue = parseSessionRandId(sessionRandIdString);

        const std::string xorUsername = xorToHex(userName, sessionRandIdValue + 1u);
        const std::string hashXorUsername = sha1Hex(xorUsername);

        const std::string xorPasswordHash = xorToHex(passwordHash, sessionRandIdValue);
        const std::string hashXorPasswordHash = sha1Hex(xorPasswordHash);
        const std::string checkPasswordFunction = m_isHttps ? "CheckPasswordServer" : "CheckPassword";
        parameters = "Command&Type=Session&SubType=" + checkPasswordFunction
            + "&Name=" + hashXorUsername
            + "&Password=" + hashXorPasswordHash;

        const std::string sessionRandIdString2 = getResponseString(parameters);
        if (isFailResponse(sessionRandIdString2))
        {
            m_lastError = trim(sessionRandIdString2);
            logMessage(LogLevel::Error, "ControllerApi::login"
                        , "Failed to authenticate session: " + m_lastError);
            return false;
        }
        if (!m_isHttps)
        {
            const std::uint32_t sessionRandIdValue2 = parseSessionRandId(sessionRandIdString2);
            const std::string xorPasswordHash2 = xorToHex(passwordHash, sessionRandIdValue2);
            const std::string hashXorPasswordHash2 = sha1Hex(xorPasswordHash2);
            if (hashXorPasswordHash2.size() < 16)
            {
                throw std::runtime_error("Invalid session key length");
            }
            std::ranges::copy_n(hashXorPasswordHash2.begin(), 16, m_aesKey.begin());
        }
        m_loggedIn = true;
        return true;
    }

    // Log out of the WX controller.
    bool ControllerApi::logout()
    {
        if (!m_loggedIn)
        {
            return true;
        }
        bool closed = false;
        try
        {
            const std::string parameters = "Command&Type=Session&SubType=CloseSession";
            getResponseString(parameters, Reply::Raw);
            closed = true;
        }
        catch (const std::exception& e)
        {
            logMessage(LogLevel::Warning, "ControllerApi::logout", e.what());
        }
        catch (...)
        {
            // Never let an exception escape: the destructor calls logout().
            logMessage(LogLevel::Warning, "ControllerApi::logout", "Unknown error while closing the session");
        }
        clearSession();
        return closed;
    }

    // Restart all expansion modules.
    bool ControllerApi::restartAllModules()
    {
        return sendCommand(CommandType::Modules, "Restart", {{"Module", "All"}});
    }

    // Restart the controller.
    bool ControllerApi::restartController()
    {
        try
        {
            if (!sendCommand(CommandType::RestartController))
                return false;
        }
        catch (const std::exception& e)
        {
            // The controller can drop connection before replying "OK".
            logMessage(LogLevel::Warning, "ControllerApi::restartController", e.what());
        }
        clearSession();
        return true;
    }

    // Download a database backup.
    std::optional<std::string> ControllerApi::downloadBackup()
    {
        const std::string parameters = "Request&Type=Backup";
        std::string response = getResponseString(parameters, Reply::Raw);
        if (isFailResponse(response))
        {
            m_lastError = trim(response);
            logMessage(LogLevel::Error, "ControllerApi::downloadBackup", m_lastError);
            return std::nullopt;
        }
        return response;
    }

    // Request a response table from the controller.
    std::optional<KeyValueList> ControllerApi::sendRequest(const RequestType type, const std::string& subType, const KeyValueList& params)
    {
        const std::string typeString = toString(type);
        const std::string parameters = buildParameters("Request", typeString, subType, params);
        const std::string response = trim(getResponseString(parameters));
        if (isFailResponse(response))
        {
            m_lastError = response;
            logMessage(LogLevel::Error, "ControllerApi::sendRequest"
                        , typeString + " " + subType + ": " + m_lastError);
            return std::nullopt;
        }
        return parseQueryString(response);
    }

    // Send a command to the controller.
    bool ControllerApi::sendCommand(const CommandType type, const std::string& subType, const KeyValueList& params)
    {
        const std::string typeString = toString(type);
        const std::string parameters = buildParameters("Command", typeString, subType, params);
        const std::string response = trim(getResponseString(parameters));
        if (!response.starts_with("OK"))
        {
            m_lastError = response;
            logMessage(LogLevel::Error, "ControllerApi::sendCommand",
                       typeString + " " + subType + ": " + m_lastError);
            return false;
        }
        return true;
    }
} // ICT
