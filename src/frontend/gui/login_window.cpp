//
// Created by hohaia on 26/10/2026.
//

#include "login_window.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QFormLayout>
#include <QFont>
#include <QGroupBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

#include "controller_api.h"
#include "logger.h"
#include "workflow.h"

namespace ict::gui
{
    // Build the login form and connect its buttons.
    LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent)
    {
        // Input widgets; the layouts below take ownership of them.
        auto *httpsCheck = new QCheckBox;
        auto *hostTextBox = new QLineEdit;
        auto *portNumBox = new QLineEdit;
        auto *userTextBox = new QLineEdit;
        auto *passwordTextBox = new QLineEdit;
        auto *loginButton = new QPushButton("Login");
        auto *exitButton = new QPushButton("Exit");

        // Hints and input rules.
        hostTextBox->setPlaceholderText("* Required");
        portNumBox->setPlaceholderText("80");
        portNumBox->setValidator(new QIntValidator(1, 65535, portNumBox));
        userTextBox->setPlaceholderText("* Required");
        passwordTextBox->setPlaceholderText("Enter Password");
        passwordTextBox->setEchoMode(QLineEdit::Password);

        // Layouts: a form for the rows, with horizontal rows for the host fields and the buttons.
        auto *layout    = new QFormLayout(this);
        auto *hostRow   = new QHBoxLayout;
        auto *loginRow  = new QHBoxLayout;
        auto *errorInfoBox = new QGroupBox("Login Failure Responses.");

        // Bold group box title (the contents are made non-bold again below).
        QFont titleFont = errorInfoBox->font();
        titleFont.setBold(true);
        errorInfoBox->setFont(titleFont);

        hostRow->addWidget(new QLabel("https://"));
        hostRow->addWidget(httpsCheck);
        hostRow->addWidget(new QLabel("IP/Domain:"));
        hostRow->addWidget(hostTextBox);
        hostRow->addWidget(new QLabel("Port:"));
        hostRow->addWidget(portNumBox);

        loginRow->addWidget(loginButton);
        loginRow->addWidget(exitButton);

        // Table of the controller's login FAIL responses (from overview-authentication-client.md).
        auto *errorInfoLabel = new QLabel(
            "<table border='1' cellspacing='0' cellpadding='4' style='border-collapse: collapse'>"
            "<tr><th align='left'>Response</th><th align='left'>Meaning</th></tr>"
            "<tr><td>FAIL</td><td>Wrong username or password</td></tr>"
            "<tr><td>FAIL 5</td><td>More than 3 bad attempts, wait 5 s</td></tr>"
            "<tr><td>FAIL 60</td><td>More than 6 bad attempts, wait 60 s</td></tr>"
            "<tr><td>FAIL. No valid operator login found.</td><td>Controller was defaulted; change the admin login first</td></tr>"
            "</table>");
        errorInfoLabel->setWordWrap(true);
        QFont tableFont = errorInfoLabel->font();
        tableFont.setBold(false);   // Explicitly set, so it overrides the group box's bold.
        errorInfoLabel->setFont(tableFont);

        auto *errorInfoLayout = new QVBoxLayout(errorInfoBox);   // Layout inside the group box.
        errorInfoLayout->addWidget(errorInfoLabel);

        // Put the rows together, top to bottom.
        layout->addRow(hostRow);
        layout->addRow("Username:", userTextBox);
        layout->addRow("Password:", passwordTextBox);
        layout->addRow(loginRow);
        layout->addRow(errorInfoBox);

        connect(exitButton, &QPushButton::clicked, this, &QWidget::close); // Exit closes the window, which quits the app.

        // Match the port hint to the protocol's default port.
        connect(httpsCheck, &QCheckBox::toggled, this, [portNumBox](bool checked)
        {
            portNumBox->setPlaceholderText(checked ? "443" : "80");
        });

        connect(loginButton, &QPushButton::clicked, this,
            [this, httpsCheck, hostTextBox, portNumBox, userTextBox, passwordTextBox]
            {
                // Check the input before trying to connect.
                const QString host = hostTextBox->text().trimmed();
                QUrl url;
                url.setHost(host, QUrl::StrictMode);
                if (!url.isValid() || url.host().isEmpty())
                {
                    QMessageBox::warning(this, "Login", "Enter a valid IP or domain");
                    return;
                }
                if (userTextBox->text().trimmed().isEmpty())
                {
                    QMessageBox::warning(this, "Login", "Enter a username");
                    return;
                }

                try
                {
                    // Log in; wx logs out again when it goes out of scope at the end of the try.
                    core::ControllerApi wx(host.toStdString()
                                         , httpsCheck->isChecked()
                                         , portNumBox->text().toStdString());

                    const auto result = core::loginAndFetchSettings(wx
                                              , userTextBox->text().toStdString()
                                              , passwordTextBox->text().toStdString());
                    // A wrong password is a result, not an exception.
                    if (result != core::LoginResult::LoggedIn)
                    {
                        QMessageBox::warning(this, "Login Failed", QString::fromStdString(wx.lastError()));
                        return;
                    }
                    QMessageBox::information(this, "Login", "Logged in");
                }
                // Connection errors are thrown; catch them here because Qt slots must not throw.
                catch (const std::exception &e)
                {
                    const QString errorMsg = e.what();
                    core::logMessage(ict::core::LogLevel::Error, "LoginWindow", e.what());
                    QMessageBox::warning(this, "Login Failed", errorMsg);
                }
            });
    }
}
