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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "serial.h"
#include "csvfilemange.h"
#include "httpclient.h"
#include "public.h"
#include "inputwindow.h"
#include "QSimpleUpdater/toolupdate.h"
#include "LogModule/openlog.h"

#include <QMainWindow>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refresh_window();
    void refresh_process_window();
    void success_process_window();
    void window_limits();
    void port_list_noticSlot(QList<QString> portList);
    void network_drops(int network_status);
    void start_burning();
    void refresh_info(QString code);
    void dataReadNoticSlot(QByteArray recvData);
    void ATCMDSend(QString data);
    bool offline_mode_burning();
    bool online_mode_burning();
    void recvCheckTimeOut();
    void process_inputwindow_signal(QString code, QString product_name, QString product_key);
    QString compress_ds(QString ds);
    QString strings_handle();

    void on_pushButton_mode_clicked();
    void on_pushButton_burn_clicked();
    void on_pushButton_input_clicked();
    void on_hideButton_clicked();
    void on_action_about_triggered();
    void on_action_doc_triggered();
    void on_action_update_triggered();
    void languageChange(int mode);
    void on_action_chinese_triggered();
    void on_action_english_triggered();
    void on_action_CN_triggered();
    void on_action_EU_triggered();
    void on_action_US_triggered();
    void on_action_dumpLog_triggered();
    void on_action_userLog_triggered();

private:
    Ui::MainWindow *ui;
    csvFileMange *g_csvFile = NULL;
    qSerial *qserial;
    QTimer *recvTimer;
    HttpClient *httpclient;
    PublicWork *pwork = NULL;
    toolUpdate *updateVersion;
    QImage *img_offline;
    QImage *img_online;
    InputWindow *inputwindow;
    OpenLog *openlog;

signals:
    void startBurningSignal();
    void scanNetlinkSignal(int);
};

extern struct workStatus status;
extern struct workInfo d_info;
extern QTranslator *main_translator;

#endif // MAINWINDOW_H
