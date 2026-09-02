//
// Created by hohaia on 23/08/2026.
//

#ifndef UNTITLED_CONTROLLERAPI_H
#define UNTITLED_CONTROLLERAPI_H
#include <string>
#include <array>

#include "httplib.h"

namespace ict
{
    class ControllerAPI
    {
    private:
        //static functions
        static std::string cleanAddress(const std::string& address);
        static std::string xorFn(const std::string& inputString, const std::uint32_t& num);
        //functions
        bool shouldEncrypt(const std::string& parameters) const;
        httplib::Client createClient() const;
        std::string getResponseString(std::string& parameters);
        std::string encrypt(const std::string& parameters) const;
        std::string decrypt(const std::string& parameters) const;
        //variables
        std::array<std::uint8_t, 16> m_aesKey{};
        const std::string m_domain;
        const std::string m_path;
        const bool m_isHttps;
        httplib::Client m_client;
    protected:
    public:
        //constructors and deconstructors
        ControllerAPI(const std::string& address, const bool https)
            : m_domain(cleanAddress(address))
            , m_path("/PRT_CTRL_DIN_ISAPI.dll?")
            , m_isHttps(https)
            , m_client(createClient())
        {
        }
        ~ControllerAPI()
            = default;
        //functions
        static std::string sha1FromString(const std::string& inputString);
        bool login(const std::string& username, const std::string& pswHash);
        bool logout();
    };
} // ICT

#endif //UNTITLED_CONTROLLERAPI_H