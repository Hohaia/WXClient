//
// Created by hohaia on 23/08/2026.
//

#ifndef WXCLIENT_CONTROLLERAPI_H
#define WXCLIENT_CONTROLLERAPI_H

#include <array>
#include <cstdint> // IWYU pragma: keep
#include <httplib.h>
#include <optional>
#include <string>

#include "protocol.h"

namespace ict::core
{
    // The communication layer to a specific ICT Protege WX controller.
    class ControllerApi
    {
        enum class Reply {Decrypt, Raw}; // Raw: logout and backup replies are never encrypted.

        //variables
        static constexpr const char* m_path = "/PRT_CTRL_DIN_ISAPI.dll?";
        std::array<std::uint8_t, 16> m_aesKey{};
        std::string m_sessionCookie;
        std::string m_lastError;
        const std::string m_clientSessionId;
        const std::string m_host;
        int m_sequenceNumber = 0;
        const bool m_isHttps;
        bool m_needsClientSessionId = false;
        bool m_loggedIn = false;
        httplib::Client m_client;

    public:
        std::optional<KeyValueList> m_settings;
        std::string m_serialNumber;

        //constructors and deconstructors
        ControllerApi(const std::string& host, const bool isHttps)
            : m_clientSessionId(generateSessionId())
            , m_host(cleanAddress(host))
            , m_isHttps(isHttps)
            , m_client(createClient())
        {
        }
        ~ControllerApi();
        ControllerApi(const ControllerApi&) = delete;
        ControllerApi& operator=(const ControllerApi&) = delete;
        ControllerApi(ControllerApi&&) = delete;
        ControllerApi& operator=(ControllerApi&&) = delete;

    private:
        // Functions.
        [[nodiscard]] static std::string cleanAddress(const std::string& address);
        [[nodiscard]] static std::string xorToHex(const std::string& inputString,
                                                  std::uint32_t xorNumber);
        [[nodiscard]] static bool isFailResponse(const std::string& response);
        [[nodiscard]] static std::uint32_t parseSessionRandId(const std::string& response);
        [[nodiscard]] static std::string parseCookiePair(const std::string& setCookieHeader);
        [[nodiscard]] static std::string generateSessionId();
        [[nodiscard]] static std::string buildParameters(const std::string& kind,
                                                         const std::string& type,
                                                         const std::string& subType,
                                                         const KeyValueList& params);

        [[nodiscard]] httplib::Client createClient() const;
        [[nodiscard]] std::string buildRequestString(const std::string& requestString) const;
        std::string getResponseString(std::string requestString, Reply reply = Reply::Decrypt);
        [[nodiscard]] std::string encrypt(const std::string& requestString) const;
        [[nodiscard]] std::string decrypt(const std::string& encryptedResponse) const;
        void clearSession();

    public:
        //functions
        [[nodiscard]] const std::string& lastError() const;
        bool login(const std::string& userName,
                   const std::string& passwordHash);
        bool logout();
        bool restartAllModules();
        bool restartController();
        [[nodiscard]] std::optional<std::string> downloadBackup();
        [[nodiscard]] std::optional<KeyValueList> sendRequest(RequestType type,
                                                              const std::string& subType,
                                                              const KeyValueList& params = {});
        bool sendCommand(CommandType type,
                         const std::string& subType = "",
                         const KeyValueList& params = {});
    };
}

#endif //WXCLIENT_CONTROLLERAPI_H