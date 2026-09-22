//
// Created by hohaia on 23/08/2026.
//

#include "controller_api.h"

#include <algorithm>
#include <bitset>
#include <charconv>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>

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

    // Calculate the XOR between a string and xorNumber.
    std::string ControllerApi::xorToHex(const std::string& inputString,const std::uint32_t& xorNumber)
    {
        const std::bitset<32> bits(xorNumber);
        const std::string key = bits.to_string();
        std::size_t offset = key.size();
        std::ostringstream oss;
        for (const unsigned char ch : inputString)
        {
            offset = (offset == 0) ? (key.size() - 8u) : (offset - 8u);
            const std::string byteKey = key.substr(offset, 8u);
            const auto byteVal = static_cast<unsigned int>(std::stoi(byteKey, nullptr, 2));

            oss << std::uppercase
                << std::hex
                << std::setw(2)
                << std::setfill('0')
                << (static_cast<unsigned int>(ch) ^ byteVal);
        }
        return oss.str();
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

    // Create the http client.
    httplib::Client ControllerApi::createClient() const
    {
        std::string protocol;
        if (m_isHttps)
        {
            protocol = "https";
        }
        else
        {
            protocol = "http";
        }
        const std::string cliDomain = protocol + "://" + m_host + "/";
        httplib::Client cli(cliDomain);
        cli.enable_server_certificate_verification(false);
        cli.set_connection_timeout(5);
        cli.set_read_timeout(5);
        cli.set_write_timeout(5);
        return cli;
    }

    // Build the request requestString depending on WX Controllers firmware version.
    std::string ControllerApi::buildRequestString(std::string& requestString) const
    {
        if (!m_needsClientSessionId)
        {
            return requestString;
        }
        if (requestString.starts_with("Command&Type=Session&SubType=InitSession")
            || requestString.starts_with("Command&Type=Session&SubType=CheckPassword"))
        {
            requestString = requestString + "&SessionID=" + m_clientSessionId;
            return requestString;
        }
        const std::string seqNum = std::to_string(m_sequenceNumber);
        requestString = requestString + "&Sequence=" + seqNum;

        return requestString;
    }

    // Perform a POST request and read the response as a string.
    std::string ControllerApi::getResponseString(std::string& requestString)
    {
        const bool skipDecryption = requestString.starts_with("Command&Type=Session&SubType=CloseSession") ||
                                    requestString.starts_with("Request&Type=Backup");
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
        if (!skipDecryption && m_loggedIn && !m_isHttps)
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

    /* PUBLIC FUNCTIONS */
    // Close any open session before the object goes away.
    ControllerApi::~ControllerApi()
    {
        logout();
    }

    // Create a sha1 checksum from a string.
    std::string ControllerApi::sha1Hex(const std::string& inputString)
    {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digestLength = 0;
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP context");
        }
        if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1
            || EVP_DigestUpdate(ctx, inputString.data(), inputString.size()) != 1
            || EVP_DigestFinal_ex(ctx, digest, &digestLength) != 1)
        {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to compute SHA-1");
        }
        EVP_MD_CTX_free(ctx);
        std::ostringstream oss;
        oss << std::uppercase << std::hex;
        for (unsigned int i = 0; i < digestLength; ++i)
        {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(digest[i]);
        }
        return oss.str();
    }

    // Return the message from the most recent failed call, for a frontend to display.
    const std::string &ControllerApi::lastError() const
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
            std::string parameters = "Command&Type=Session&SubType=CloseSession";
            getResponseString(parameters);
            closed = true;
        }
        catch (...)
        {
        }
        m_loggedIn = false;
        m_sessionCookie.clear();
        m_aesKey = {};
        return closed;
    }

    // Request a response table from the controller.
    std::optional<ResponseTable> ControllerApi::sendRequest(const std::string& type, const std::string& subType)
    {
        std::string parameters;
        switch (toRequestType(type))
        {
            case RequestType::List:
            case RequestType::Detail:
            case RequestType::Events:
            case RequestType::Status:
            case RequestType::Health:
            case RequestType::Modules:
            case RequestType::DuplicateCheck:
            case RequestType::System:
                parameters = "Request&Type=" + type + "&SubType=" + subType;
                break;
            case RequestType::Backup:
                parameters = "Request&Type=" + type;
                break;
            default:
                throw std::runtime_error("Unknown request type: " + type);
        }
        const std::string response = trim(getResponseString(parameters));
        if (isFailResponse(response))
        {
            m_lastError = response;
            logMessage(LogLevel::Error, "ControllerApi::sendRequest"
                        , type + " " + subType + ": " + m_lastError);
            return std::nullopt;
        }
        return parseQueryString(response);
    }

    // Download a database backup.
    std::optional<std::string> ControllerApi::downloadBackup()
    {
        std::string parameters = "Request&Type=Backup";
        std::string response = getResponseString(parameters);
        if (isFailResponse(response))
        {
            m_lastError = trim(response);
            logMessage(LogLevel::Error, "ControllerApi::downloadBackup", m_lastError);
            return std::nullopt;
        }
        return response;
    }

    // Send a command to the controller.
    bool ControllerApi::sendCommand(const std::string& type, const std::string& subType, const std::string& recId
                                  , const std::string& command, const std::string& data1, const std::string& data2)
    {
        std::string parameters;
        switch (toCommandType(type))
        {
            case CommandType::Submit:
            case CommandType::Modules:
            case CommandType::Restore:
                parameters = "Command&Type=" + type + "&SubType=" + subType;
                break;
            case CommandType::Delete:
                parameters = "Command&Type=" + type + "&SubType=" + subType + "&RecId=" + recId;
                break;
            case CommandType::Control:
                parameters = "Command&Type=" + type + "&SubType=" + subType + "&RecId=" + recId + "&Command=" + command;
                if (!data1.empty()) {parameters += "&Data1=" + data1;}
                if (!data2.empty()) {parameters += "&Data2=" + data2;}
                break;
            case CommandType::RestartController:
                parameters = "Command&Type=" + type;
                break;
            default:
                throw std::runtime_error("Unknown command type: " + type);
        }
        const std::string response = trim(getResponseString(parameters));
        if (!response.starts_with("OK"))
        {
            m_lastError = response;
            logMessage(LogLevel::Error, "ControllerApi::sendCommand"
                        , type + " " + subType + ": " + m_lastError);
            return false;
        }
        return true;
    }
} // ICT
