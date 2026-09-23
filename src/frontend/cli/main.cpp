#include <exception>
#include <iostream>

#include "console.h"
#include "logger.h"
#include "session.h"

int main()
{
    try
    {
        return ict::runCli();
    }
    catch (const std::exception& e)
    {
        ict::logMessage(ict::LogLevel::Error, "main", e.what());
        ict::printError(e.what());
        return 1;
    }
}
