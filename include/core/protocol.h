//
// Created by hohaia on 27/09/2026.
//

#ifndef WXCLIENT_PROTOCOL_H
#define WXCLIENT_PROTOCOL_H

#include <string>
#include <utility>
#include <vector>

namespace ict::core
{
    // Ordered key=value pairs, as in a request or reply ({"RecId", "3"} <-> "RecId=3").
    using KeyValueList = std::vector<std::pair<std::string, std::string>>;

    // Request&Type=<RequestType> ("List").
    enum class RequestType {List, Detail, Events, Status, Health, Modules, DuplicateCheck, System};
    // Command&Type=<CommandType> ("Control").
    enum class CommandType {Submit, Delete, Modules, Control, RestartController};
    // Request&Type=Events&SubType=<EventRequest> ("Latest").
    enum class EventRequest {Latest, Previous, Next, Update};

    [[nodiscard]] std::string encodeStringValue(const std::string& value);
    [[nodiscard]] std::string decodeStringValue(const std::string& str);
    [[nodiscard]] KeyValueList parseQueryString(const std::string& query);
    [[nodiscard]] std::string toString(RequestType type);
    [[nodiscard]] std::string toString(CommandType type);
    [[nodiscard]] std::string toString(EventRequest request);
}

#endif //WXCLIENT_PROTOCOL_H
