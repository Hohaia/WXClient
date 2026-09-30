//
// Created by hohaia on 30/09/2026.
//

#ifndef WXCLIENT_STATUS_CODE_TABLES_H
#define WXCLIENT_STATUS_CODE_TABLES_H

#include <array>

#include "area_status_tables.h"
#include "door_status_tables.h"

namespace ict::core
{
    // Status decoding for each controller table.
    constexpr std::array statusCodeTables{
        // StatusCodeTable {"GXT_ANALOGEXPANDERS_TBL", xStatusFields},
        StatusCodeTable {"GXT_AREAS_TBL", areaStatusFields},
        // StatusCodeTable {"GXT_CONTROLLERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_DATAVALUES_TBL", xStatusFields},
        StatusCodeTable{"GXT_DOORS_TBL", doorStatusFields},
        // StatusCodeTable {"GXT_FLOORS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_INPUTEXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_INPUTS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_KEYPADS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PGMEXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PGMS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PROGRAMMABLEFUNCTIONS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_READEREXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_READERUNITS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_SCHEDULES_TBL", xStatusFields},
        // StatusCodeTable {"GXT_SERVICES_TBL", xStatusFields},
        // StatusCodeTable {"GXT_TROUBLEINPUTS_TBL", xStatusFields},
        // StatusCodeTable {"AREA_ZONE_STATUS", xStatusFields},
        // StatusCodeTable {"GXT_MODULES_TBL", xStatusFields},
        // StatusCodeTable {"ModuleFirmwareUpdate", xStatusFields},
    };
}

#endif //WXCLIENT_STATUS_CODE_TABLES_H
