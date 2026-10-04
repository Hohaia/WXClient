//
// Created by hohaia on 29/09/2026.
//

#ifndef WXCLIENT_CONTROL_COMMAND_TABLES_H
#define WXCLIENT_CONTROL_COMMAND_TABLES_H

#include "area_control_tables.h"
#include "door_control_tables.h"
#include "input_control_tables.h"
#include "output_control_tables.h"

namespace ict::core
{
    // Control commands for each controller table.
    constexpr std::array controlCommandTables{
        ControlCommandTable{"GXT_DOORS_TBL", doorControlCommands},
        ControlCommandTable{"GXT_AREAS_TBL", areaControlCommands},
        ControlCommandTable{"GXT_PGMS_TBL", outputControlCommands},
        ControlCommandTable{"GXT_INPUTS_TBL", inputControlCommands},
        // ControlCommandTable{"GXT_DATAVALUES_TBL", dataValuesControlCommands},
        // ControlCommandTable{"GXT_SERVICES_TBL", servicesControlCommands},
        // ControlCommandTable{"GXT_PROGRAMMABLEFUNCTIONS_TBL", programmableFunctionsControlCommands},
        // ControlCommandTable{"RTC", rtcControlCommands},
        // ControlCommandTable{"GXT_FLOORS_TBL", floorsControlCommands},
        // ControlCommandTable{"RESET_ANTIPASSBACK", resetAntipassbackControlCommands}
    };
}

#endif //WXCLIENT_CONTROL_COMMAND_TABLES_H
