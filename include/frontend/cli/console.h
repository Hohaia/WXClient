//
// Created by hohaia on 03/09/2026.
//
#ifndef WXCLIENT_CONSOLE_H
#define WXCLIENT_CONSOLE_H

#include <iostream>
#include <span>
#include <string>

#include "helpers.h"

namespace ict
{
    struct StaticMenuItem
    {
        int key;
        std::string_view label;
    };

    std::string readLine(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    std::string readPassword(const std::string& prompt);
    void printTable(const ResponseTable& table);

    // Print a menu to the console.
    template <class T>
    int printMenu(std::span<const T> menu, std::string_view title)
    {
        std::cout << "\033[2J\033[H"; // Clear console, set cursor to top-left.
        std::cout << "<<<<<" << title << ">>>>>\n\n";
        for (const auto& item : menu)
        {
            std::cout << item.key << ". " << item.label << "\n";
        }
        int choice = 0;
        while (true)
        {
            bool validChoice = false;
            std::string input = readLine("\nSelect: ");
            try
            {
                choice = std::stoi(input);
            }
            catch (const std::exception&)
            {
                std::cout << "Please enter a valid choice.\n";
                continue;
            }
            for (const auto& item : menu)
            {
                if (choice == item.key)
                {
                    validChoice = true;
                    break;
                }
            }
            if (!validChoice)
            {
                std::cout << "Please enter a valid choice.\n";
                continue;
            }
            break;
        }
        return choice;
    }
}

#endif //WXCLIENT_CONSOLE_H
