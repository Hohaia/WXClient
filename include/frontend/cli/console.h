//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <iostream>
#include <span>
#include <string>

#include "controller_api.h"
#include "protocol.h"

namespace ict
{
    // A single cli menu line ([key: "1", label: "Doors"] -> 1. Doors).
    struct StaticMenuItem
    {
        std::string key;
        std::string_view label;
        bool gapBefore = false;
    };

    std::string readLine(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    std::string readPassword(const std::string& prompt);
    void printTable(const KeyValueList& table);
    void printError(const std::string& message);
    void waitForEnter();

    // Print a menu to the console.
    template <class T>
    std::string printMenu(const ControllerApi& wx, std::span<const T> menu, std::string_view title)
    {
        std::cout << "\033[2J\033[H"; // Clear console, set cursor to top-left.
        std::cout << "<<<<<" << title << ">>>>>\n";
        std::cout << "Serial Number: " << wx.m_serialNumber << "\n\n";
        for (const auto& item : menu)
        {
            if (item.gapBefore)
                std::cout << "\n";
            std::cout << item.key << ". " << item.label << "\n";
        }
        while (true)
        {
            std::string choice = readLine("\nSelect: ");
            if (std::ranges::any_of(menu, [&choice](const T& item) { return item.key == choice; }))
                return choice;
            std::cout << "Please enter a valid choice.\n";
        }
    }
}

#endif //WXCLIENT_CONSOLE_H
