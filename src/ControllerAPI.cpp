//
// Created by hohaia on 23/08/2026.
//
#include <openssl/rand.h>
#include <stdexcept>

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
    std::string ControllerAPI::sha1FromString(std::string inputString)
    {
        std::string result;

        return result;
    }

    //calculate the XOR between a string and num
    std::string ControllerAPI::xorFn(std::string inputString, int num)
    {
        std::string result;

        return result;
    }

    //determine if outgoing requests need encryption
    bool ControllerAPI::shouldEncrypt(const std::string& parameters) const
    {
        if(m_isHttps == true)
        {
            return false;
        }
        if(parameters.starts_with("Command&Type=Session&SubType=InitSession") || parameters.starts_with("Command&Type=Session&SubType=CheckPassword"))
        {
            return false;
        }
        return true;
    }

    //create the http client
    httplib::Client ControllerAPI::createClient() const
    {
        std::string httpsYN;
        if(m_isHttps == true)
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
        return(cli);
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
        std::string response = result->body;
        if(encryptParameters)
        {
            response = decrypt(response);
        }
        return response;
    }

    //encrypt a string
    std::string ControllerAPI::encrypt(const std::string& parameters) const
    {
        std::string result;

        return result;
    }
    //decrypt a string

    std::string ControllerAPI::decrypt(const std::string& parameters) const
    {
        std::string result;

        return result;
    }

    /* PUBLIC FUNCTIONS*/
    //login to the WX controller
    bool login(std::string username, std::string pswHash)
    {
        return false;
    }
} // ICT