//
// Created by hohaia on 23/08/2026.
//
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ControllerAPI.h"

namespace ict
{
    /* PRIVATE FUNCTIONS*/
    //clean up the host address (remove any leading "http://, https://, www.)
    std::string ControllerAPI::cleanAddress(const std::string& address)
    {
        std::string s = address;
        for (const auto& prefix : { std::string("https://"), std::string("http://"), std::string("www.") }) {
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

    //calculate the XOR between a string and num
    std::string ControllerAPI::xorFn(const std::string& inputString,const std::uint32_t& num)
    {
        const std::bitset<32> bits(num);
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

    //convert a hex string to a byte string
    std::vector<std::uint8_t> ControllerAPI::hexToBytes(const std::string& hexStr)
    {
        std::vector<std::uint8_t> bytes;
        bytes.reserve(hexStr.size() / 2);

        for (std::size_t i = 0; i < hexStr.size(); i += 2)
        {
            const std::string byteStr = hexStr.substr(i, 2);
            bytes.push_back(static_cast<std::uint8_t>(std::stoi(byteStr, nullptr, 16)));
        }

        return bytes;
    }

    //convert a byte string to a hex string
    std::string ControllerAPI::toHex(const std::vector<std::uint8_t>& bytes)
    {
        std::ostringstream oss;
        for (const auto& byte : bytes)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << byte;
        }
        return oss.str();
    }

    //determine if outgoing requests need encryption
    bool ControllerAPI::shouldEncrypt(const std::string& parameters) const
    {
        if (m_isHttps)
        {
            return false;
        }
        if (parameters.starts_with("Command&Type=Session&SubType=InitSession")
            || parameters.starts_with("Command&Type=Session&SubType=CheckPassword"))
        {
            return false;
        }
        return true;
    }

    //create the http client
    httplib::Client ControllerAPI::createClient() const
    {
        std::string httpsYN;
        if (m_isHttps)
        {
            httpsYN = "https";
        }
        else
        {
            httpsYN = "http";
        }
        const std::string cliDomain = httpsYN + "://" + m_domain + "/";
        httplib::Client cli(cliDomain);
        cli.enable_server_certificate_verification(false);
        return cli;
    }

    //perform a POST request and read the response as a string
    std::string ControllerAPI::getResponseString(std::string& parameters)
    {
        const bool encryptParameters = shouldEncrypt(parameters);
        if (encryptParameters)
        {
            parameters = encrypt(parameters);
        }
        const std::string request = m_path + parameters;
        auto result = m_client.Post(request);
        if (!result || result->status != 200)
        {
            throw std::runtime_error("Failed to POST to controller");
        }
        std::string response = result->body;
        if (encryptParameters)
        {
            response = decrypt(response);
        }
        return response;
    }

    //encrypt a string
    std::string ControllerAPI::encrypt(const std::string& parameters) const
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
        std::vector<std::uint8_t> encryptedBytes(parameters.size() + 16);
        int encryptedLength = 0;
        if (EVP_EncryptUpdate(ctx, encryptedBytes.data(), &encryptedLength
                                , reinterpret_cast<const unsigned char*>(parameters.data())
                                , static_cast<int>(parameters.size())) != 1)
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

    //decrypt a string
    std::string ControllerAPI::decrypt(const std::string& parameters) const
    {
        if (parameters.size() < 32)
        {
            throw std::runtime_error("Invalid encrypted payload");
        }
        const std::string ivStr = parameters.substr(0, 32);
        const std::string encryptedStr = parameters.substr(32);
        const auto iv = hexToBytes(ivStr);
        const auto encryptedBytes = hexToBytes(encryptedStr);
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
            throw std::runtime_error("Failed to finalize encryption");
        }
        EVP_CIPHER_CTX_free(ctx);
        decryptedLength += finalLength;
        return {reinterpret_cast<const char*>(decryptedBytes.data())
                    , static_cast<std::string::size_type>(decryptedLength)};
    }

    /* PUBLIC FUNCTIONS*/
    //create a sha1 checksum from a string
    std::string ControllerAPI::sha1FromString(const std::string& inputString)
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

    //login to the WX controller
    bool ControllerAPI::login(const std::string& username, const std::string& pswHash)
    {
        std::string parameters = "Command&Type=Session&SubType=InitSession";
        const std::string sessionRandIdString = getResponseString(parameters);
        const auto sessionRandIdValue = static_cast<std::uint32_t>(std::stoul(sessionRandIdString));
        const std::string xorUsername = xorFn(username, sessionRandIdValue + 1u);
        const std::string hashXorUsername = sha1FromString(xorUsername);
        const std::string xorPasswordHash = xorFn(pswHash, sessionRandIdValue);
        const std::string hashXorPasswordHash = sha1FromString(xorPasswordHash);
        const std::string checkPasswordFunction = m_isHttps ? "CheckPasswordServer" : "CheckPassword";
        parameters = "Command&Type=Session&SubType=" + checkPasswordFunction
            + "&Name=" + hashXorUsername
            + "&Password=" + hashXorPasswordHash;
        const std::string sessionRandIdString2 = getResponseString(parameters);
        if (sessionRandIdString2.rfind("FAIL", 0) == 0)
        {
            std::cout << "Error in authentication: " << sessionRandIdString2 << std::endl;
            return false;
        }
        if (!m_isHttps)
        {
            const auto sessionRandIdValue2 = static_cast<std::uint32_t>(std::stoul(sessionRandIdString2));
            const std::string xorPasswordHash2 = xorFn(pswHash, sessionRandIdValue2);
            const std::string hashXorPasswordHash2 = sha1FromString(xorPasswordHash2);
            const std::string sessionKeyString = hashXorPasswordHash2.substr(0, 16);
            if (sessionKeyString.size() != 16) {
                throw std::runtime_error("Invalid session key length");
            }

            std::ranges::copy(sessionKeyString, m_aesKey.begin());
        }
        return true;
    }

    //log out of the WX controller
    bool ControllerAPI::logout()
    {
        try
        {
            std::string parameters = "Command&Type=Session&SubType=CloseSession";
            getResponseString(parameters);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
} // ICT