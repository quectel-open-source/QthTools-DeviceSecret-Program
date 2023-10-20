/*
Copyright (C) 2023  Quectel Wireless Solutions Co.,Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef INPUTWINDOW_H
#define INPUTWINDOW_H

#include "httpclient.h"

#include <QWidget>

namespace Ui {
class InputWindow;
}

class InputWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InputWindow(int cloud, QWidget *parent = nullptr);
    ~InputWindow();
    void close_input_window();

private slots:
    void on_pushButton_clicked();

private:
    Ui::InputWindow *ui;
    HttpClient *httpclient;
    int status = 0;

signals:
    void inputResultSignal(QString, QString, QString);
    void iloginAgainSignal();
};

#endif // INPUTWINDOW_H
