//
// Created by hohaia on 13/09/2026.
//

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget loginWindow;

    auto* httpsCheck = new QCheckBox;
    auto* hostTextBox = new QLineEdit;
    auto* portNumBox = new QLineEdit;
    auto* userTextBox = new QLineEdit;
    auto* passwordTextBox = new QLineEdit;
    auto* loginButton = new QPushButton("Login");
    auto* exitButton = new QPushButton("Exit");

    hostTextBox->setPlaceholderText("* Required");
    portNumBox->setPlaceholderText("80");
    portNumBox->setValidator(new QIntValidator(1, 65535, portNumBox));
    userTextBox->setPlaceholderText("* Required");
    passwordTextBox->setPlaceholderText("Enter Password");
    passwordTextBox->setEchoMode(QLineEdit::Password);

    auto* layout = new QFormLayout(&loginWindow);
    auto* hostRow = new QHBoxLayout;
    auto* loginRow = new QHBoxLayout;

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

    QObject::connect(exitButton, &QPushButton::clicked, &loginWindow, &QWidget::close);
    QObject::connect(httpsCheck, &QCheckBox::toggled, [portNumBox](bool checked)
    {
        portNumBox->setPlaceholderText(checked ? "443" : "80");
    });
    QObject::connect(loginButton, &QPushButton::clicked,
        [httpsCheck, hostTextBox, portNumBox, userTextBox]
        {
            const QString host = hostTextBox->text().trimmed();
            QUrl url;
            url.setHost(host, QUrl::StrictMode);
            if (!url.isValid() || url.host().isEmpty())
            {
                QMessageBox::warning(nullptr, "Login", "Enter a valid IP or domain");
                return;
            }
            if (userTextBox->text().trimmed().isEmpty())
            {
                QMessageBox::warning(nullptr, "Login", "Enter a username");
                return;
            }

            const QString protocol = httpsCheck->isChecked() ? "https://" : "http://";
            const QString address = portNumBox->text().isEmpty() ? host : host + ":" + portNumBox->text();

            QMessageBox::information(nullptr, "Login", "Host: " + protocol + address);
        });

    loginWindow.show();

    return app.exec();
}
