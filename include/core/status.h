//
// Created by hohaia on 24/09/2026.
//

#ifndef WXCLIENT_STATUS_H
#define WXCLIENT_STATUS_H

#include <string>
#include <string_view>
#include <vector>

namespace ict::core
{
    // A single decoded field (name: "Position", text: "Left Open").
    struct DecodedField
    {
        std::string_view name;
        std::string text;
        bool isEmpty = false;
    };

    [[nodiscard]] std::vector<DecodedField> decodeStatus(std::string_view tableName, std::string_view rawValue);
}

#endif //WXCLIENT_STATUS_H
