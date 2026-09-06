//
// Created by hohaia on 23/08/2026.
//

#ifndef UNTITLED_CONTROLLERAPI_H
#define UNTITLED_CONTROLLERAPI_H
#include <array>
#include <cstdint>
#include <map>
#include <string>

#include "httplib.h"

namespace ict
{
    class ControllerAPI
    {
        //variables
        std::array<std::uint8_t, 16> m_aesKey{};
        std::string m_sessionCookie;
        const std::string m_host;
        const std::string m_path;
        const std::string m_sessionId;
        int m_sequenceNumber = 0;
        const bool m_isHttps;
        const bool m_useSessionId;
        bool m_loggedIn = false;
        httplib::Client m_client;
    public:
        //constructors and deconstructors
        ControllerAPI(const std::string& address, const bool https, const bool authMethod)
            : m_host(cleanAddress(address))
            , m_path("/PRT_CTRL_DIN_ISAPI.dll?")
            , m_sessionId(generateSessionId())
            , m_isHttps(https)
            , m_useSessionId(authMethod)
            , m_client(createClient())
        {
        }
        ~ControllerAPI();
        ControllerAPI(const ControllerAPI&) = delete;
        ControllerAPI& operator=(const ControllerAPI&) = delete;
        ControllerAPI(ControllerAPI&&) = delete;
        ControllerAPI& operator=(ControllerAPI&&) = delete;

    private:
        //functions
        static std::string cleanAddress(const std::string& address);
        static std::string xorToHex(const std::string& inputString, const std::uint32_t& num);
        static bool isFailResponse(const std::string& response);
        static std::uint32_t parseSessionRandId(const std::string& response);
        static std::string parseCookiePair(const std::string& setCookie);
        static std::string generateSessionId();

        bool shouldEncrypt() const;
        httplib::Client createClient() const;
        std::string buildRequestParameters(std::string& parameters) const;
        std::string sendRequest(std::string& parameters);
        std::string encrypt(const std::string& parameters) const;
        std::string decrypt(const std::string& parameters) const;

    public:
        //functions
        static std::string sha1Hex(const std::string& inputString); //used in main(), keep public:
        bool login(const std::string& username, const std::string& pswHash);
        bool logout();
        std::multimap<std::string, std::string> fetchControllerSettings();
    };
} // ICT

#endif //UNTITLED_CONTROLLERAPI_H