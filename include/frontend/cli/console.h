//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <span>
#include <string>
#include <string_view>

namespace ict::cli
{
    // A single cli menu line ([key: "1", label: "Doors"] -> 1. Doors; an empty key shows "•  label", not selectable).
    struct MenuItem
    {
        std::string key;
        std::string_view label;
        bool gapBefore = false;
    };

    std::string readLine(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    std::string readPassword(const std::string& prompt);
    void printError(const std::string& message);
    void waitForEnter();
    std::string printMenu(std::span<const MenuItem> menu, std::string_view title, const std::string& serialNumber);
}

#endif //WXCLIENT_CONSOLE_H
