//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_CONTROL_COMMAND_TYPES_H
#define WXCLIENT_CONTROL_COMMAND_TYPES_H

#include <span>
#include <string_view>

namespace ict::core
{
    // A single control command (code: 2, label: "Activate (timed)", data1: "Activation Time").
    struct ControlCommand
    {
        int code;
        std::string_view label;
        std::string_view data1 = {}; // Empty = no Data1 needed.
        std::string_view data2 = {}; // Empty = no Data2 needed.
    };

    // All the control commands for a single controller table (name: "GXT_DOORS_TBL", commands: doorControlCommands).
    struct ControlCommandTable
    {
        std::string_view name;
        std::span<const ControlCommand> commands;
    };
}

#endif //WXCLIENT_CONTROL_COMMAND_TYPES_H
