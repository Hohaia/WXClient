#include <exception>
#include <iostream>

#include "session.h"

int main()
{
    try
    {
        return ict::runCli();
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
}
