//
// Created by hohaia on 13/09/2026.
//

#include "workflow.h"

#include <cctype>
#include <exception>

#include "controller_api.h"
#include "helpers.h"
#include "known_certificates.h"

namespace ict::core
{
    namespace
    {
        // Fetch a table's record list, or nullopt if the List request failed.
        std::optional<std::vector<RecordEntry>> fetchRecordList(ControllerApi& wx, const std::string& tableName)
        {
            const std::optional<KeyValueList> list = wx.sendRequest(RequestType::List, tableName);
            if (!list)
                return std::nullopt;   // Already logged by ControllerApi.
            std::vector<RecordEntry> items;
            for (const auto& [recId, label] : *list)
                items.emplace_back(recId, label);
            return items;
        }
    }

    // Log in and fetch the controller's settings; an untrusted certificate is reported, not thrown.
    LoginResult loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password)
    {
        // Load this host's saved certificate once; a retry after trustCertificate() keeps the new one.
        if (wx.trustedFingerprint().empty())
        {
            if (const auto saved = loadTrustedFingerprint(wx.host()))
                wx.trustFingerprint(*saved);
        }
        try
        {
            if (!wx.login(userName, toLower(sha1Hex(password))))
                return LoginResult::Failed;
        }
        catch (const std::exception&)
        {
            if (wx.untrustedFingerprint().empty())
                throw;   // Any other transport error still reaches main().
            return wx.trustedFingerprint().empty() ? LoginResult::UntrustedCertificate
                                                   : LoginResult::CertificateChanged;
        }
        wx.fetchSettings();   // Logged in even if this fails; the caller checks wx.settings().
        return LoginResult::LoggedIn;
    }

    // Trust the certificate the controller just presented, now and for future logins; false if it couldn't be saved.
    bool trustCertificate(ControllerApi& wx)
    {
        const std::string fingerprint = wx.untrustedFingerprint();
        wx.trustFingerprint(fingerprint);
        return saveTrustedFingerprint(wx.host(), fingerprint);
    }

    // Look up a record list, fetching and caching it on first use; nullptr if the fetch failed.
    const std::vector<RecordEntry>* getRecordList(ControllerApi& wx, RecordListCache& cache, const std::string& tableName)
    {
        if (const auto it = cache.find(tableName); it != cache.end())
            return &it->second;
        auto items = fetchRecordList(wx, tableName);
        if (!items)
            return nullptr;   // Not cached, so the next visit retries.
        return &cache.emplace(tableName, std::move(*items)).first->second;
    }

    // Fetch the live statuses for a list, keyed by RecId; nullopt if the request failed.
    std::optional<StatusMap> fetchStatuses(ControllerApi& wx, const std::string& tableName)
    {
        const std::optional<KeyValueList> response = wx.sendRequest(RequestType::Status, tableName);
        if (!response)
            return std::nullopt;   // Already logged by ControllerApi.
        StatusMap statuses;
        for (const auto& [key, value] : *response)
        {
            const auto digits = key.find_first_of("0123456789");
            if (digits == std::string::npos)
                continue;
            statuses.emplace(key.substr(digits), value);   // "Door12" -> "12"
        }
        return statuses;
    }

    // Fetch event descriptions ("Latest", "Previous", "Next": 20 at a time; "Update": all since the last request).
    std::optional<std::vector<std::string>> fetchEvents(ControllerApi& wx, EventRequest request)
    {
        const std::optional<KeyValueList> response = wx.sendRequest(RequestType::Events, toString(request));
        if (!response)
            return std::nullopt;   // Already logged by ControllerApi.
        std::vector<std::string> events;
        for (const auto& [key, value] : *response)
        {
            // Keep "Event12", skip "EventCodes12".
            if (key.size() <= 5 || !key.starts_with("Event") || !std::isdigit(static_cast<unsigned char>(key[5])))
                continue;
            events.emplace_back(trim(value));   // Descriptions can end in a space.
        }
        return events;
    }
}
