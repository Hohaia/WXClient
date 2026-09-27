//
// Created by hohaia on 27/09/2026.
//

#ifndef WXCLIENT_PROTOCOL_H
#define WXCLIENT_PROTOCOL_H

#include <string>
#include <utility>
#include <vector>

namespace ict
{
    using KeyValueList = std::vector<std::pair<std::string, std::string>>;

    enum class RequestType {List, Detail, Events, Status, Health, Modules, DuplicateCheck, System};
    enum class CommandType {Submit, Delete, Modules, Control, RestartController};

    [[nodiscard]] std::string encodeStringValue(const std::string& value);
    [[nodiscard]] std::string decodeStringValue(const std::string& str);
    [[nodiscard]] KeyValueList parseQueryString(const std::string& query);
    [[nodiscard]] std::string toString(RequestType type);
    [[nodiscard]] std::string toString(CommandType type);
}

#endif //WXCLIENT_PROTOCOL_H
