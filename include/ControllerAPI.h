//
// Created by hohaia on 23/08/2026.
//

#ifndef UNTITLED_CONTROLLERAPI_H
#define UNTITLED_CONTROLLERAPI_H
#include <string>
#include <array>

#include "httplib.h"

namespace ict {
    class ControllerAPI {
    private:
        //variables
        const std::array<std::uint8_t, 16> m_aesKey = std::array<std::uint8_t, 16>{};
        const std::string m_domain;
        const std::string m_path;
        const bool m_isHttps = false;
        httplib::Client m_client = httplib::Client("");
    protected:
    public:
        //constructors and deconstructors
        ControllerAPI()
            = default;
        ControllerAPI(const std::string& address, const bool https)
            : m_aesKey(generateAesKey())
            , m_domain(cleanAddress(address))
            , m_isHttps(https)
            , m_client(createClient())
            , m_path("/PRT_CTRL_DIN_ISAPI.dll?")
        {
        }
        ~ControllerAPI()
            = default;

        //functions
    private:
        static std::string cleanAddress(const std::string& address);
        std::array<std::uint8_t, 16> generateAesKey();
        bool shouldEncrypt(const std::string& parameters) const;
        httplib::Client createClient() const;
        std::string getResponseString(std::string& parameters);
        std::string encrypt(const std::string& parameters) const;
        std::string decrypt(const std::string& parameters) const;
        static std::string sha1FromString(std::string inputString);
        static std::string xorFunc(std::string inputString, int num);
    public:
        bool login(std::string username, std::string pswHash);
    };
} // ICT

#endif //UNTITLED_CONTROLLERAPI_H