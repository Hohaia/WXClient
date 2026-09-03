//
// Created by hohaia on 03/09/2026.
//
#include <iostream>
#include <optional>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

#include "console.h"
#include "helpers.h"

namespace
{
    //restore the terminal's original echo state on scope exit
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

    //discard the newline left behind by a preceding formatted read
    void discardPendingNewline()
    {
        if (std::cin.peek() == '\n')
        {
            std::cin.ignore();
        }
    }

    //read a whole line or fail
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

//write a prompt and read a trimmed line of input
std::string readLine(const std::string& prompt)
{
    std::cout << prompt << std::flush;
    discardPendingNewline();
    return trim(readLineOrThrow());
}

//write a prompt and read a y/n answer, repeating until one is given
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
        std::cout << "Please answer y or n." << std::endl;
    }
}

//write a prompt and read a line without echoing it to the terminal
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
        discardPendingNewline();
        password = readLineOrThrow();
    }

    std::cout << std::endl;
    return password;
}