//
// Created by hohaia on 03/09/2026.
//

#include "console.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

#include "logger.h"
#include "helpers.h"

namespace ict::cli
{
    namespace
    {
        // Restore the terminal's original echo state on scope exit.
        class EchoGuard
        {
        public:
            explicit EchoGuard(const termios& original)
                : m_original(original)
            {
            }
            ~EchoGuard()
            {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_original);
            }
            EchoGuard(const EchoGuard&) = delete;
            EchoGuard& operator=(const EchoGuard&) = delete;
            EchoGuard(EchoGuard&&) = delete;
            EchoGuard& operator=(EchoGuard&&) = delete;
        private:
            termios m_original;
        };

        // Read a whole line or fail.
        std::string readLineOrThrow()
        {
            std::string line;
            if (!std::getline(std::cin, line))
            {
                throw std::runtime_error("Unexpected end of input");
            }
            return line;
        }
    }

    // Write a prompt and read a trimmed line of input.
    std::string readLine(const std::string& prompt)
    {
        std::cout << prompt << std::flush;
        return core::trim(readLineOrThrow());
    }

    // Write a prompt and read a y/n answer, repeating until one is given.
    bool readYesNo(const std::string& prompt)
    {
        while (true)
        {
            const std::string answer = readLine(prompt);
            if (answer == "y" || answer == "Y")
            {
                return true;
            }
            if (answer == "n" || answer == "N")
            {
                return false;
            }
            std::cout << "Please answer y or n.\n";
        }
    }

    // Write a prompt and read a line without echoing it to the terminal.
    std::string readPassword(const std::string& prompt)
    {
        std::cout << prompt << std::flush;

        termios original{};
        const bool isTerminal = isatty(STDIN_FILENO) == 1
            && tcgetattr(STDIN_FILENO, &original) == 0;

        std::string password;
        {
            std::optional<EchoGuard> guard;
            if (isTerminal)
            {
                guard.emplace(original);
                termios quiet = original;
                quiet.c_lflag &= ~static_cast<tcflag_t>(ECHO);
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &quiet);
            }
            password = readLineOrThrow();
        }
        if (isTerminal)
        {
            std::cout << "\n";
        }
        return password;
    }

    // Print an error message and the filepath to 'logs.csv'.
    void printError(const std::string& message)
    {
        std::cout << "\n" << message << "\n";
        std::cout << "\nLog file: " << core::logFilePath().string() << "\n";
    }

    // Wait for 'Enter' input before continuing.
    void waitForEnter()
    {
        readLine("\nPress [Enter] to continue...");
    }

    // Print a menu to the console and read a valid choice ("exit" after confirming a logout).
    std::string printMenu(std::span<const MenuItem> menu, std::string_view title, const std::string& serialNumber)
    {
        std::cout << "\033[2J\033[H"; // Clear console, set cursor to top-left.
        std::cout << "<<<<<" << title << ">>>>>\n";
        std::cout << "Serial Number: " << serialNumber << "\n\n";
        for (const auto& item : menu)
        {
            if (item.gapBefore)
                std::cout << "\n";
            if (item.key.empty())
                std::cout << "\u2022  " << item.label << "\n"; // Info (read only) line (not selectable).
            else
                std::cout << item.key << ". " << item.label << "\n";
        }
        std::cout << "\nType \"exit\" to log out.\n";
        while (true)
        {
            std::string choice = core::toLower(readLine("\nSelect: "));
            if (choice == "exit")
            {
                if (readYesNo("\nAre you sure you want to log out? (y/n): "))
                    return choice;
                continue;
            }
            if (!choice.empty() && std::ranges::any_of(menu, [&choice](const MenuItem& item) { return item.key == choice; }))
                return choice;
            std::cout << "Please enter a valid choice.\n";
        }
    }
}
