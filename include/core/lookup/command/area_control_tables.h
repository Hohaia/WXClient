//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_AREA_CONTROL_TABLES_H
#define WXCLIENT_AREA_CONTROL_TABLES_H

#include <array>

#include "control_command_types.h"

namespace ict::core
{
    // 'Area' from 'control.md' (as a ControlCommand {code, label} pair).
    constexpr std::array areaControlCommands{
        ControlCommand{0, "Disarm"},
        ControlCommand{1, "Disarm (24hr)"},
        ControlCommand{2, "Disarm (all)"},
        ControlCommand{3, "Arm"},
        ControlCommand{4, "Force Arm"},
        ControlCommand{5, "Arm (instant)"},
        ControlCommand{6, "Force Arm (instant)"},
        ControlCommand{7, "Enable Walk Test"},
        ControlCommand{8, "Disable Walk Test"},
        ControlCommand{9, "Silence Alarm"},
        ControlCommand{10, "Stay Arm"},
        ControlCommand{11, "Arm (24hr)"}
    };
}

#endif //WXCLIENT_AREA_CONTROL_TABLES_H
