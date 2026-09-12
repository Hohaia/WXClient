//
// Created by hohaia on 23/08/2026.
//

#ifndef UNTITLED_CONTROLLERAPI_H
#define UNTITLED_CONTROLLERAPI_H
#include <array>
#include <cstdint>
#include <string>
#include "httplib.h"

#include "helpers.h"

namespace ict
{
    class ControllerAPI
    {
        //variables
        std::array<std::uint8_t, 16> m_aesKey{};
        std::string m_sessionCookie;
        const std::string m_clientSessionId;
        const std::string m_host;
        const std::string m_path;
        int m_sequenceNumber = 0;
        const bool m_isHttps;
        bool m_needsClientSessionId = false;
        bool m_loggedIn = false;
        httplib::Client m_client;
    public:
        //constructors and deconstructors
        ControllerAPI(const std::string& host, const bool isHttps)
            : m_clientSessionId(generateSessionId())
            , m_host(cleanAddress(host))
            , m_path("/PRT_CTRL_DIN_ISAPI.dll?")
            , m_isHttps(isHttps)
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
        static std::string xorToHex(const std::string& inputString, const std::uint32_t& xorKey);
        static bool isFailResponse(const std::string& response);
        static std::uint32_t parseSessionRandId(const std::string& response);
        static std::string parseCookiePair(const std::string& setCookieHeader);
        static std::string generateSessionId();

        bool shouldEncrypt() const;
        httplib::Client createClient() const;
        std::string buildRequestString(std::string& requestString) const;
        std::string getResponseString(std::string& requestString);
        std::string encrypt(const std::string& requestString) const;
        std::string decrypt(const std::string& encryptedResponse) const;

    public:
        //functions
        static std::string sha1Hex(const std::string& inputString); //used in main(), keep public:
        bool login(const std::string& userName, const std::string& passwordHash);
        bool logout();
        bool command(const std::string& type, const std::string& subType);
        ResponseTable request(const std::string& type, const std::string& subType);
    };
} // ICT

#endif //UNTITLED_CONTROLLERAPI_H