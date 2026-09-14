//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <span>
#include <string>
#include "helpers.h"

namespace ict
{
    struct MenuItem;

    std::string readLine(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    std::string readPassword(const std::string& prompt);
    void printTable(const ResponseTable& table);
    int printMenu(std::span<const MenuItem>);
}

#endif //WXCLIENT_CONSOLE_H
