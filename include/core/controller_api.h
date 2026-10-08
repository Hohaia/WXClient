//
// Created by hohaia on 23/08/2026.
//

#ifndef WXCLIENT_CONTROLLERAPI_H
#define WXCLIENT_CONTROLLERAPI_H

#include <array>
#include <cstdint>  // IWYU pragma: keep
#include <ctime>    // IWYU pragma: keep
#include <httplib.h>
#include <optional>
#include <string>

#include "protocol.h"

namespace ict::core
{
    // The communication layer to a specific ICT Protege WX controller.
    class ControllerApi
    {
        enum class Reply {Decrypt, Raw}; // Raw: logout, backup and event export replies are never encrypted.

        // Variables.
        static constexpr const char* m_path = "/PRT_CTRL_DIN_ISAPI.dll?";
        static constexpr std::time_t requestTimeout = 5;        // Seconds; normal requests.
        static constexpr std::time_t downloadTimeout = 300;     // Seconds; exports can take longer to build.
        std::array<std::uint8_t, 16> m_aesKey{};
        std::string m_sessionCookie;
        std::string m_lastError;
        const std::string m_clientSessionId;
        const std::string m_host;
        int m_sequenceNumber = 0;
        const bool m_isHttps;
        bool m_needsClientSessionId = false;
        bool m_loggedIn = false;
        std::optional<KeyValueList> m_settings;
        std::string m_serialNumber;
        std::string m_trustedFingerprint;                       // SHA-256 of the cert the user accepted; empty = none yet.
        std::string m_untrustedFingerprint;                     // Set when the controller presents a cert that isn't trusted.
        httplib::Client m_client;                               // Keep last: createClient() reads m_host and m_isHttps.

    public:
        // Constructors and destructors.
        ControllerApi(const std::string& host, const bool isHttps, const std::string& port = "")
            : m_clientSessionId(generateSessionId())
            , m_host(buildAddress(host, port))
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
        [[nodiscard]] static std::string buildAddress(const std::string& address, const std::string& port);
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
        [[nodiscard]] httplib::Client createClient();
        [[nodiscard]] httplib::SSLVerifierResponse verifyCertificate(httplib::tls::session_t session);
        [[nodiscard]] std::string buildRequestString(const std::string& requestString) const;
        std::string getResponseString(std::string requestString, Reply reply = Reply::Decrypt);
        [[nodiscard]] std::optional<std::string> downloadFile(const std::string& parameters);
        [[nodiscard]] std::string encrypt(const std::string& requestString) const;
        [[nodiscard]] std::string decrypt(const std::string& encryptedResponse) const;
        void clearSession();

    public:
        // Functions.
        [[nodiscard]] const std::string& lastError() const;
        [[nodiscard]] const std::optional<KeyValueList>& settings() const;
        [[nodiscard]] const std::string& serialNumber() const;
        [[nodiscard]] const std::string& untrustedFingerprint() const;
        [[nodiscard]] const std::string& host() const;
        [[nodiscard]] const std::string& trustedFingerprint() const;
        void trustFingerprint(const std::string& fingerprint);
        bool fetchSettings();
        bool login(const std::string& userName,
                   const std::string& passwordHash);
        bool logout();
        bool restartAllModules();
        bool restartController();
        [[nodiscard]] std::optional<std::string> downloadBackup();
        [[nodiscard]] std::optional<std::string> downloadEventLog(const std::string& startDate = "",
                                                                  const std::string& endDate = "");
        [[nodiscard]] std::optional<KeyValueList> sendRequest(RequestType type,
                                                              const std::string& subType,
                                                              const KeyValueList& params = {});
        bool sendCommand(CommandType type,
                         const std::string& subType = "",
                         const KeyValueList& params = {});
    };
}

#endif //WXCLIENT_CONTROLLERAPI_H