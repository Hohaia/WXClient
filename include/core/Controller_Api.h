//
// Created by hohaia on 23/08/2026.
//

#ifndef UNTITLED_CONTROLLERAPI_H
#define UNTITLED_CONTROLLERAPI_H

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include "httplib.h"

#include "helpers.h"

namespace ict
{
    class Controller_Api
    {
        //variables
        std::array<std::uint8_t, 16> m_aesKey{};
        std::string m_sessionCookie;
        std::string m_lastError;
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
        Controller_Api(const std::string& host, const bool isHttps)
            : m_clientSessionId(generateSessionId())
            , m_host(cleanAddress(host))
            , m_path("/PRT_CTRL_DIN_ISAPI.dll?")
            , m_isHttps(isHttps)
            , m_client(createClient())
        {
        }
        ~Controller_Api();
        Controller_Api(const Controller_Api&) = delete;
        Controller_Api& operator=(const Controller_Api&) = delete;
        Controller_Api(Controller_Api&&) = delete;
        Controller_Api& operator=(Controller_Api&&) = delete;

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
        static std::string sha1Hex(const std::string& inputString); //used in src/workflow.cpp, keep public:
        const std::string& lastError() const;
        bool login(const std::string& userName, const std::string& passwordHash);
        bool logout();
        std::optional<ResponseTable> sendRequest(const std::string& type, const std::string& subType);
        bool sendCommand(const std::string& type, const std::string& subType = "", const std::string& recId = ""
                       , const std::string& command = "", const std::string& data1 = "", const std::string& data2 = "");
    };
} // ICT

#endif //UNTITLED_CONTROLLERAPI_H