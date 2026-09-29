//
// Created by hohaia on 29/09/2026.
//

#ifndef WXCLIENT_CONTROL_H
#define WXCLIENT_CONTROL_H

#include <span>
#include <string_view>

#include "control_command_tables.h"


namespace ict
{
    [[nodiscard]] std::span<const ControlCommand> findControlCommands(std::string_view name);
}


#endif //WXCLIENT_CONTROL_H
