//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <map>
#include <string>

std::string readLine(const std::string& prompt);
bool readYesNo(const std::string& prompt);
std::string readPassword(const std::string& prompt);
void printTable(const std::multimap<std::string, std::string>& table);

#endif //WXCLIENT_CONSOLE_H
