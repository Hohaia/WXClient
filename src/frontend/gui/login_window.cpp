//
// Created by hohaia on 26/10/2026.
//

#include "login_window.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

namespace ict::gui
{
    LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent)
    {
        auto *httpsCheck = new QCheckBox;
        auto *hostTextBox = new QLineEdit;
        auto *portNumBox = new QLineEdit;
        auto *userTextBox = new QLineEdit;
        auto *passwordTextBox = new QLineEdit;
        auto *loginButton = new QPushButton("Login");
        auto *exitButton = new QPushButton("Exit");

        hostTextBox->setPlaceholderText("* Required");
        portNumBox->setPlaceholderText("80");
        portNumBox->setValidator(new QIntValidator(1, 65535, portNumBox));
        userTextBox->setPlaceholderText("* Required");
        passwordTextBox->setPlaceholderText("Enter Password");
        passwordTextBox->setEchoMode(QLineEdit::Password);

        auto *layout    = new QFormLayout(this);
        auto *hostRow   = new QHBoxLayout;
        auto *loginRow  = new QHBoxLayout;

        hostRow->addWidget(new QLabel("https://"));
        hostRow->addWidget(httpsCheck);
        hostRow->addWidget(new QLabel("IP/Domain:"));
        hostRow->addWidget(hostTextBox);
        hostRow->addWidget(new QLabel("Port:"));
        hostRow->addWidget(portNumBox);

        loginRow->addWidget(loginButton);
        loginRow->addWidget(exitButton);

        layout->addRow(hostRow);
        layout->addRow("Username:", userTextBox);
        layout->addRow("Password:", passwordTextBox);
        layout->addRow(loginRow);

        connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
        connect(httpsCheck, &QCheckBox::toggled, this, [portNumBox](bool checked)
        {
            portNumBox->setPlaceholderText(checked ? "443" : "80");
        });
        connect(loginButton, &QPushButton::clicked, this,
            [this, httpsCheck, hostTextBox, portNumBox, userTextBox]
            {
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

                const QString protocol = httpsCheck->isChecked() ? "https://" : "http://";
                const QString address = portNumBox->text().isEmpty() ? host : host + ":" + portNumBox->text();

                QMessageBox::information(this, "Login", "Host: " + protocol + address);
            });
    }
}
