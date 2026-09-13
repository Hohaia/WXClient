//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_LOGGER_H
#define WXCLIENT_LOGGER_H

#include <string>

namespace ict
{
    enum class LogLevel {Info, Warning, Error};

    //append one row to logs.csv: timestamp,level,source,message.
    void logMessage(LogLevel level, const std::string& source, const std::string& message);
}

#endif //WXCLIENT_LOGGER_H
