//
// Created by hohaia on 13/09/2026.
//

#include <exception>

#include <QApplication>

#include "logger.h"
#include "login_window.h"

int main(int argc, char *argv[])
{
    try
    {
        QApplication app(argc, argv);

        ict::gui::LoginWindow loginWindow;
        loginWindow.show();

        return app.exec();
    }
    catch (const std::exception &e)
    {
        ict::core::logMessage(ict::core::LogLevel::Error, "main", e.what());

        return 1;
    }
}
