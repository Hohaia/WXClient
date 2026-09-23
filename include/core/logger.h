//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_LOGGER_H
#define WXCLIENT_LOGGER_H

#include <filesystem>
#include <string>

namespace ict
{
    enum class LogLevel {Info, Warning, Error};

    std::filesystem::path logFilePath();
    void logMessage(LogLevel level, const std::string& source, const std::string& message);
}

#endif //WXCLIENT_LOGGER_H
