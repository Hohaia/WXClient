//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <string>

std::string readLine(const std::string& prompt);
bool readYesNo(const std::string& prompt);
std::string readPassword(const std::string& prompt);

#endif //WXCLIENT_CONSOLE_H
