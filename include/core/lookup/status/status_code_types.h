//
// Created by hohaia on 24/09/2026.
//

#ifndef WXCLIENT_STATUS_CODE_TYPES_H
#define WXCLIENT_STATUS_CODE_TYPES_H

#include <span>
#include <string_view>

namespace ict::core
{
    // How to decode a single field, e.g. for doorPositionStatus (Value: 3 -> "Left Open", Bits: 3 -> 011, Raw: 3 as is).
    enum class StatusType { Value, Bits, Raw };

    // A single status code and a label for it (code: 0, label: "Locked").
    struct StatusCodeLabel
    {
        int code; // Value: the code. Bits: the bit number.
        std::string_view label;
    };

    // One part of a record's status and how to decode it (name: "Lock", type: StatusType::Value, labels: doorLockStatusCodes).
    struct StatusCodeField
    {
        std::string_view name;
        StatusType type;
        std::span<const StatusCodeLabel> labels;
    };

    // All the status fields for one controller table (tableName: "GXT_DOORS_TBL", fields: doorStatusFields).
    struct StatusCodeTable
    {
        std::string_view name;
        std::span<const StatusCodeField> fields;
    };
}

#endif //WXCLIENT_STATUS_CODE_TYPES_H
