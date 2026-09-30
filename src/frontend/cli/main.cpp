#include <exception>
#include <iostream>

#include "console.h"
#include "logger.h"
#include "session.h"

int main()
{
    try
    {
        return ict::cli::runCli();
    }
    catch (const std::exception& e)
    {
        ict::core::logMessage(ict::core::LogLevel::Error, "main", e.what());
        ict::cli::printError(e.what());
        return 1;
    }
}
