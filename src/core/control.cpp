//
// Created by hohaia on 29/09/2026.
//

#include "control.h"

#include <algorithm>

#include "control_command_tables.h"

namespace ict::core
{
    // Find the control commands for a table (empty if the table has none).
    std::span<const ControlCommand> findControlCommands(std::string_view name)
    {
        const auto it = std::ranges::find_if(controlCommandTables,
                                     [&](const ControlCommandTable& table)
                                     {return table.name == name;});
        if (it == controlCommandTables.end())
            return {};
        return it->commands;
    }
}
