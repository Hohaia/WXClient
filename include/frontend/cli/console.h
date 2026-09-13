//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <string>
#include "helpers.h"

namespace ict
{
    std::string readLine(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    std::string readPassword(const std::string& prompt);
    void printTable(const ResponseTable& table);
    void mainMenu();
}

#endif //WXCLIENT_CONSOLE_H
