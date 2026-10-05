//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_WORKFLOW_H
#define WXCLIENT_WORKFLOW_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "controller_api.h"

namespace ict::core
{
    // A single entry from a controller table (recId: "0", label: "Entry Door" [door @ address 0]).
    struct RecordEntry
    {
        std::string recId;
        std::string label;
    };

    using RecordListCache = std::unordered_map<std::string, std::vector<RecordEntry>>;      // Cache of a table's contents ("GXT_DOORS_TBL": {"0", "Entry Door"}, {"1", "Side Door"}, {...}).
    using StatusMap = std::unordered_map<std::string, std::string>;                         // recID -> raw status, (e.g. "0" -> "23,3,0").

    enum class LoginResult {LoggedIn, Failed, UntrustedCertificate, CertificateChanged};    // How a login attempt ended.

    LoginResult loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password);
    bool trustCertificate(ControllerApi& wx);
    const std::vector<RecordEntry>* getRecordList(ControllerApi& wx, RecordListCache& cache, const std::string& tableName);
    std::optional<StatusMap> fetchStatuses(ControllerApi& wx, const std::string& tableName);
    std::optional<std::vector<std::string>> fetchEvents(ControllerApi& wx, EventRequest request);
}

#endif //WXCLIENT_WORKFLOW_H
