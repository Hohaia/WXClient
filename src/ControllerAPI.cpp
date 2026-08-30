//
// Created by hohaia on 23/08/2026.
//
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <bitset>

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

    //generate a random 16byte AES key
    std::array<std::uint8_t, 16> ControllerAPI::generateAesKey()
    {
        std::array<std::uint8_t, 16> key{};
        if (RAND_bytes(key.data(), static_cast<int>(key.size())) != 1)
        {
            throw std::runtime_error("Failed to generate AES key");
        }
        return key;
    }

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
        for (unsigned int i = 0; i < digestLength; ++i)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
        }
        return oss.str();
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
        std::string result;
        //TODO
        return result;
    }
    //decrypt a string

    std::string ControllerAPI::decrypt(const std::string& parameters) const
    {
        std::string result;
        //TODO
        return result;
    }

    /* PUBLIC FUNCTIONS*/
    //login to the WX controller
    bool ControllerAPI::login(const std::string& username, const std::string& pswHash)
    {
        //TODO
        return false;
    }
} // ICT