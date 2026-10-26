//
// Created by hohaia on 26/10/2026.
//

#ifndef WXCLIENT_LOGIN_WINDOW_H
#define WXCLIENT_LOGIN_WINDOW_H

#include <QWidget>

namespace ict::gui
{
    class LoginWindow : public QWidget
    {
        Q_OBJECT

    public:
        explicit LoginWindow(QWidget *parent = nullptr);
    };
}

#endif //WXCLIENT_LOGIN_WINDOW_H
