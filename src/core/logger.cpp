//
// Created by hohaia on 13/09/2026.
//

#include "logger.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace ict
{
    namespace
    {
        //wrap a field in quotes
        std::string csvField(const std::string& field)
        {
            std::string escaped;
            escaped.reserve(field.size() + 2);
            escaped.push_back('"');
            for (const char ch : field)
            {
                if (ch == '"') {escaped.push_back('"'); }
                escaped.push_back(ch);
            }
            escaped.push_back('"');
            return escaped;
        }

        std::string levelName(const LogLevel level)
        {
            switch (level)
            {
                case LogLevel::Info: return "INFO";
                case LogLevel::Warning: return "WARNING";
                case LogLevel::Error: return "ERROR";
            }
            return "UNKNOWN";
        }

        std::string timeStamp()
        {
            const auto now = std::chrono::system_clock::now();
            const std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_r(&t, &tm);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        //set the directory for logs.csv, regardless of the process's cwd
        std::filesystem::path logDirectory()
        {
            #ifdef _WIN32
            #error "Windows support: resolve %LOCALAPPDATA%\\wxclient here once console.cpp no longer depends on termios."
            #else
            const char* xdgState = std::getenv("XDG_STATE_HOME");
            const char* home = std::getenv("HOME");
            const std::filesystem::path base = (xdgState && *xdgState)
                ? std::filesystem::path(xdgState)
                : std::filesystem::path(home ? home : ".") / ".local" / "state";
            return base / "wxclient";
            #endif
        }
    }

    std::filesystem::path logFilePath()
    {
        const std::filesystem::path logDir = logDirectory();
        std::error_code ec;
        std::filesystem::create_directories(logDir, ec);   // A failure shows up as the ofstream failing to open.
        return logDir / "logs.csv";
    }


    //append one row to logs.csv: timestamp,level,source,message
    void logMessage(LogLevel level, const std::string& source, const std::string& message)
    {
        const std::filesystem::path path = logFilePath();
        std::error_code ec;
        const auto size = std::filesystem::file_size(path, ec);
        const bool needsHeader = ec || size == 0;   // ec is set when the file doesn't exist yet.
        std::ofstream file(path, std::ios::app);
        if (!file)
        {
            return; //nowhere to report a logging failure; drop it rather than throw from a log call
        }
        if (needsHeader)
        {
            file << "timestamp,level,source,message\n";
        }
        file << csvField(timeStamp()) << ','
             << csvField(levelName(level)) << ','
             << csvField(source) << ','
             << csvField(message) << '\n';
    }
}
