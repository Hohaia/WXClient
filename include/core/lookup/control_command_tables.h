//
// Created by hohaia on 29/09/2026.
//

#ifndef WXCLIENT_CONTROL_COMMAND_TABLES_H
#define WXCLIENT_CONTROL_COMMAND_TABLES_H

#include <array>
#include <span>
#include <string_view>

namespace ict
{
    // A single control command (code: 1, label: "Unlock").
    struct ControlCommand
    {
        int code;
        std::string_view label;
    };

    // All the control commands for a single controller table (name: "GXT_DOORS_TBL", commands: doorControlCommands).
    struct ControlCommandTable
    {
        std::string_view name;
        std::span<const ControlCommand> commands;
    };

    // 'Doors' from 'control.md' (as a ControlCommand {code, label} pair).
    constexpr std::array doorControlCommands{
        ControlCommand{0, "Lock"},
        ControlCommand{1, "Unlock"},
        ControlCommand{2, "Unlock (latched)"},
        ControlCommand{3, "Lockdown (allow entry)"},
        ControlCommand{4, "Lockdown (allow exit)"},
        ControlCommand{5, "Lockdown (allow entry and exit)"},
        ControlCommand{6, "Clear Lockdown"},
        ControlCommand{7, "Cancel Conditional Exception"},
        ControlCommand{8, "Restore Conditional Exception"},
        ControlCommand{9, "Lockdown (full)"},
        ControlCommand{11, "Reset Door Duress"}
    };

    // Control commands for each controller table.
    constexpr std::array controlCommandTables{
        ControlCommandTable{"GXT_DOORS_TBL", doorControlCommands},
        // ControlCommandTable{"GXT_AREAS_TBL", areaControlCommands},
        // ControlCommandTable{"GXT_PGMS_TBL", outputControlCommands},
        // ControlCommandTable{"GXT_INPUTS_TBL", inputControlCommands},
        // ControlCommandTable{"GXT_DATAVALUES_TBL", dataValuesControlCommands},
        // ControlCommandTable{"GXT_SERVICES_TBL", servicesControlCommands},
        // ControlCommandTable{"GXT_PROGRAMMABLEFUNCTIONS_TBL", programmableFunctionsControlCommands},
        // ControlCommandTable{"RTC", rtcControlCommands},
        // ControlCommandTable{"GXT_FLOORS_TBL", floorsControlCommands},
        // ControlCommandTable{"RESET_ANTIPASSBACK", resetAntipassbackControlCommands}
    };
}

#endif //WXCLIENT_CONTROL_COMMAND_TABLES_H
