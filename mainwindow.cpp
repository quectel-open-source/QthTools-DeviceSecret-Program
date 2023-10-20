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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "csvfilemange.h"
#include "help/feedback.h"
#include "mythead.h"

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <synchapi.h>
#include <QAbstractItemView>
#include <synchapi.h>
#include <QStandardPaths>

static QByteArray serialBuffer;
int cloud = 0;
QString dstext;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(this->width(), this->height());

    updateVersion = new toolUpdate();
    updateVersion->checkForUpdates();
    httpclient = new HttpClient();
    this->qserial = new qSerial();
    this->qserial->portListPeriodSet(1000);
    connect(this->qserial, SIGNAL(portListNoticSignal(QList<QString>)), this, SLOT(port_list_noticSlot(QList<QString>)));
    connect(this->qserial, SIGNAL(dataReadNoticSignal(QByteArray)), this, SLOT(dataReadNoticSlot(QByteArray)));
    recvTimer = new QTimer();
    connect(recvTimer, SIGNAL(timeout()), this, SLOT(recvCheckTimeOut()));

    g_csvFile->openSqlDataBase();
    pwork = new PublicWork;
    connect(this, SIGNAL(startBurningSignal()), this, SLOT(start_burning()));
    myThead *mythread = new myThead();
    mythread->start();
    connect(this, SIGNAL(scanNetlinkSignal(int)), mythread, SLOT(netlink_work(int)));
    connect(mythread, SIGNAL(networkDropsSignal(int)), this, SLOT(network_drops(int)));

    img_offline = new QImage;
    img_online = new QImage;
    img_online->load(":/jpg/online.png");
    img_offline->load(":/jpg/offline.png");
    *img_online = img_online->scaled(25,25);
    *img_offline = img_offline->scaled(25,25);
    QString workstatus = g_csvFile->findSqlData("workingmode");
    if (workstatus != NULL)
    {
        pwork->status.work_status = workstatus.toInt();
    }
    QString burncode = g_csvFile->findSqlData("burncode");
    if (burncode != NULL)
    {
        pwork->d_info.burn_code = burncode;
    }
    QString file_name = g_csvFile->findSqlData("burnfile");
    if (file_name != NULL)
    {
        pwork->d_info.file_name = file_name;
    }
    QString language = g_csvFile->findSqlData("language");
    if (language != NULL)
    {
        if (language == "english")
        {
            languageChange(1);
        }
        else
        {
            languageChange(0);
        }
    }
    QString findCloud = g_csvFile->findSqlData("cloud");
    if (findCloud == "1")
    {
        cloud = 1;
        ui->label_cloud->setText(tr("欧洲数据中心"));
    }
    else if (findCloud == "2")
    {
        cloud = 2;
        ui->label_cloud->setText(tr("美国数据中心"));
    }
    else
    {
        cloud = 0;
        ui->label_cloud->setText(tr("中国数据中心"));
    }

    refresh_window();
    ui->hideButton->setText("︾");
    ui->label_4->setVisible(false);
    ui->comboBox_stopBits->setVisible(false);
    ui->label_6->setVisible(false);
    ui->comboBox_parity->setVisible(false);
    if (pwork->status.work_status == 1)
    {
        emit scanNetlinkSignal(pwork->status.network_status);
    }
    openlog = new OpenLog();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh_window()
{
    ui->label_pn->setText("---");
    ui->label_pk->setText("---");
    pwork->d_info.product_name = "";
    pwork->d_info.product_key = "";
    ui->pushButton_burn->setDisabled(true);
    ui->pushButton_burn->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
    refresh_process_window();

    if (0 == pwork->status.work_status)
    {
        ui->pushButton_input->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        ui->pushButton_mode->setText(tr("切换到在线模式"));
        ui->pushButton_input->setText(tr("导入密钥文件"));
        ui->pushButton_burn->setText(tr("开始烧录"));
        ui->label_cloud->hide();
        QString pktext = g_csvFile->findPkData();
        qDebug()<<"product_key = "<<pktext;
        if (pktext != NULL)
        {
            pwork->d_info.product_key = pktext;
            ui->label_pn->setText("---");
            ui->label_pk->setText(pwork->d_info.product_key);
            ui->pushButton_burn->setDisabled(false);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }
        else
        {
            ui->pushButton_burn->setDisabled(true);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }

        pwork->d_info.file_name = g_csvFile->findSqlData("burnfile");
        if (pwork->d_info.file_name != NULL)
        {
            ui->label_status->setText(pwork->d_info.file_name);
            ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(0, 170, 0);");
        }
        else
        {
            ui->label_status->setText(tr("请导入密钥文件"));
            ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(255, 0, 0);");
        }
    }
    else if (1 == pwork->status.work_status)
    { 
        ui->pushButton_burn->setDisabled(true);
        ui->pushButton_mode->setText(tr("切换到离线模式"));
        ui->pushButton_input->setText(tr("输入烧录码"));
        ui->pushButton_burn->setText(tr("开始烧录"));
        ui->label_cloud->show();
        if (pwork->status.network_status == 0)
        {
            ui->label_status->setPixmap(QPixmap::fromImage(*img_offline));
            ui->pushButton_input->setDisabled(true);
            ui->pushButton_input->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
            ui->pushButton_burn->setDisabled(true);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }
        else if (pwork->status.network_status == 1)
        {
            ui->label_status->setPixmap(QPixmap::fromImage(*img_online));
            ui->pushButton_input->setDisabled(false);
            ui->pushButton_input->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
//            ui->pushButton_burn->setDisabled(false);
//            ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }

        if (pwork->d_info.burn_code != NULL)
        {
            refresh_info(pwork->d_info.burn_code);
            if (pwork->d_info.product_key == NULL || pwork->d_info.product_name == NULL)
            {
                ui->pushButton_burn->setDisabled(true);
                ui->pushButton_burn->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
                return;
            }

            ui->label_pk->setText(pwork->d_info.product_key);
            ui->label_pn->setText(pwork->d_info.product_name);
            ui->pushButton_burn->setDisabled(false);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }
        else
        {
            ui->pushButton_burn->setDisabled(true);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
        }
    }
}

void MainWindow::refresh_process_window()
{
    ui->label_1_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(194, 194, 194);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
    ui->label_1_1->setText("1");
    ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:18px;color:rgb(194, 194, 194);");
    ui->label_1_2->setText(tr("读取DeviceKey"));
    ui->label_1_1_1->setStyleSheet("font-size:23px;color:rgb(229, 229, 229);font-weight:bold;border-color:rgb(255, 255, 255);");

    ui->label_2_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(194, 194, 194);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
    ui->label_2_1->setText("2");
    ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:18px;color:rgb(194, 194, 194);");
    ui->label_2_2->setText(tr("获取DeviceSecret"));
    ui->label_2_1_1->setStyleSheet("font-size:23px;color:rgb(229, 229, 229);font-weight:bold;border-color:rgb(255, 255, 255);");

    ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(194, 194, 194);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
    ui->label_3_1->setText("3");
    ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:18px;color:rgb(194, 194, 194);");
    ui->label_3_2->setText(tr("写入DeviceSecret"));
}

void MainWindow::success_process_window()
{
    ui->label_1_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
    ui->label_1_1->setText("√");
    ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
    ui->label_1_2->setText(QString(tr("读取DeviceKey -- 成功：%1").arg(pwork->d_info.device_key)));
    ui->label_1_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");

    ui->label_2_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
    ui->label_2_1->setText("√");
    ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
    ui->label_2_2->setText(QString(tr("获取DeviceSecret -- 成功：%1").arg(dstext)));
    ui->label_2_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");

    ui->label_3_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
    ui->label_3_1->setText("√");
    ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
    ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 成功：烧录完成")));
}

void MainWindow::window_limits()
{
    if (1 == pwork->status.burn_state)
    {
        ui->comboBox_uartPort->setEnabled(false);
        ui->comboBox_uartBate->setEnabled(false);
        ui->comboBox_parity->setEnabled(false);
        ui->comboBox_stopBits->setEnabled(false);
        ui->pushButton_mode->setDisabled(true);
        ui->pushButton_input->setDisabled(true);
        ui->hideButton->setDisabled(true);
        ui->menu->setDisabled(true);
        ui->menu_2->setDisabled(true);
        ui->menu_3->setDisabled(true);
        ui->menu_4->setDisabled(true);
        ui->pushButton_burn->setText(tr("停止烧录"));
        ui->pushButton_mode->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-style:solid;border-width:1px;border-color:rgb(198, 198, 198);border-radius:3px");
        ui->pushButton_input->setStyleSheet("background-color:rgb(198, 198, 198);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
    }
    else if (0 == pwork->status.burn_state)
    {
        ui->comboBox_uartPort->setEnabled(true);
        ui->comboBox_uartBate->setEnabled(true);
        ui->comboBox_parity->setEnabled(true);
        ui->comboBox_stopBits->setEnabled(true);
        ui->pushButton_mode->setDisabled(false);
        ui->pushButton_input->setDisabled(false);
        ui->hideButton->setDisabled(false);
        ui->menu->setDisabled(false);
        ui->menu_2->setDisabled(false);
        ui->menu_3->setDisabled(false);
        ui->menu_4->setDisabled(false);
        ui->pushButton_burn->setText(tr("开始烧录"));
        ui->pushButton_mode->setStyleSheet("background-color:rgb(255, 255, 255);font-weight:bold;font-size:18px;border-style:solid;border-width:1px;border-color:rgb(81, 81, 81);border-radius:3px");
        ui->pushButton_input->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:18px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
    }
}

void MainWindow::on_pushButton_mode_clicked()
{
    if (0 == pwork->status.work_status)
    {
        qDebug()<<"当前工作模式为在线模式";
        pwork->status.work_status = 1;
        g_csvFile->setWorkSqlDate("workingmode", QString::number(pwork->status.work_status));
        emit scanNetlinkSignal(pwork->status.network_status);
        refresh_window();
        if (pwork->status.network_status == 0)
        {
            ui->label_status->setPixmap(QPixmap::fromImage(*img_offline));
            qDebug()<<"当前网络离线";
        }
        else if (pwork->status.network_status == 1)
        {
            ui->label_status->setPixmap(QPixmap::fromImage(*img_online));
            qDebug()<<"当前网络在线";
            if ("---" == ui->label_pk->text() && "---" == ui->label_pn->text())
            {
                if (pwork->d_info.burn_code == NULL && pwork->status.work_status == 1)
                {
//                        QMessageBox::information(this, "提示", "获取产品信息失败，请先导入烧录码", "确定");
                    QMessageBox box(QMessageBox::Information, tr("提示"), tr("获取产品信息失败，是否立即输入烧录码"));
                    box.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                    box.setButtonText(QMessageBox::Yes, QString(tr("确定")));
                    box.setButtonText(QMessageBox::No, QString(tr("取消")));
                    if(QMessageBox::Yes == box.exec())
                    {
                        if (pwork->status.network_status == 1)
                        {
                            on_pushButton_input_clicked();
                        }
                        else
                        {
                            QMessageBox::information(this, tr("错误"), tr("当前网络不可用"), tr("确定"));
                        }
                    }
                }
            }
        }
        return;
    }
    else if (1 == pwork->status.work_status)
    {
        qDebug()<<"当前工作模式为离线模式";
        pwork->status.work_status = 0;
        emit scanNetlinkSignal(pwork->status.network_status);
        g_csvFile->setWorkSqlDate("workingmode", QString::number(pwork->status.work_status));
        refresh_window();
        if ("---" == ui->label_pk->text() && "---" == ui->label_pn->text())
        {
//            QMessageBox::information(this, "提示", "获取产品信息失败，请先导入密钥烧录文件", "确定");
            QMessageBox box(QMessageBox::Information, tr("提示"), tr("获取产品信息失败，是否立即导入密钥烧录文件"));
            box.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
            box.setButtonText(QMessageBox::Yes, QString(tr("确定")));
            box.setButtonText(QMessageBox::No, QString(tr("取消")));
            if(QMessageBox::Yes == box.exec())
            {
                on_pushButton_input_clicked();
            }
        }
        
        if (pwork->d_info.file_name != NULL)
        {
            ui->label_status->setText(pwork->d_info.file_name);
            ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(0, 170, 0);");
        }
        else
        {
            ui->label_status->setText(tr("请导入密钥文件"));
            ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(255, 0, 0);");
        }
        return;
    }
}

void MainWindow::port_list_noticSlot(QList<QString> portList)
{
    qDebug()<<"串口列表发生变化"<<portList;
    if(portList.count() <= 0)
    {
        return;
    }

    QString currentPort = ui->comboBox_uartPort->currentText();
    ui->comboBox_uartPort->clear();
    ui->comboBox_uartPort->addItems(portList);

    if(this->qserial->serialIsOpen() && portList.contains(currentPort) == false)
    {
        this->qserial->SerialClose();
        ui->comboBox_uartPort->setEnabled(true);
        ui->comboBox_uartBate->setEnabled(true);
    }
    else if(this->qserial->serialIsOpen() == false)
    {
            ui->comboBox_uartPort->setCurrentText(portList.first());
    }
    else
    {
        ui->comboBox_uartPort->setCurrentText(currentPort);
    }

    //调整下拉列表宽度，完整显示
    int max_len = 0;
    for(int idx = 0; idx < ui->comboBox_uartPort->count(); idx ++)
    {
        if(max_len < ui->comboBox_uartPort->itemText(idx).toLocal8Bit().length())
        {
            max_len = ui->comboBox_uartPort->itemText(idx).toLocal8Bit().length();
        }
    }

    if (max_len * 12 * 0.75 < ui->comboBox_uartPort->width())
    {
        ui->comboBox_uartPort->view()->setFixedWidth(ui->comboBox_uartPort->width());
    }
    else
    {
        ui->comboBox_uartPort->view()->setFixedWidth(max_len * 12 * 0.75);
    }
}

void MainWindow::on_pushButton_burn_clicked()
{
    if (1 == pwork->status.work_status && 0 == pwork->status.network_status)
    {
        QMessageBox::information(this, tr("错误"), tr("当前网络不可用"), tr("确定"));
        return;
    }
    refresh_process_window();
    if (0 == pwork->status.burn_state)
    {
        pwork->d_info.device_key = "";
        qDebug()<<"当前烧录端口："<<ui->comboBox_uartPort->currentText();
        qDebug()<<"当前烧录波特率："<<ui->comboBox_uartBate->currentText().toInt();

        bool isopen = false;
        if (ui->hideButton->text() == "︽")
        {
            isopen = this->qserial->serialOpen(ui->comboBox_uartPort->currentText(), ui->comboBox_uartBate->currentText().toInt(), ui->comboBox_parity->currentIndex(), ui->comboBox_stopBits->currentIndex(), QSerialPort::NoFlowControl);
        }
        else if (ui->hideButton->text() == "︾")
        {
            isopen = this->qserial->serialOpen(ui->comboBox_uartPort->currentText(), ui->comboBox_uartBate->currentText().toInt(), QSerialPort::NoParity, QSerialPort::NoFlowControl);
        }

        if (true == isopen)
        {
            pwork->status.burn_state = 1;
            window_limits();
//            refresh_process_window();
            ui->label_1_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
            ui->label_1_2->setText(tr("读取DeviceKey -- 读取中..."));
            emit startBurningSignal();
        }
        else
        {
            QMessageBox::information(this, tr("警告"), tr("串口打开失败，请检查是否被占用"), tr("确定"));
            pwork->status.burn_state = 0;
            refresh_process_window();
            window_limits();
        }
    }
    else if (1 == pwork->status.burn_state)
    {
        serialBuffer.clear();
        this->qserial->SerialClose();
        pwork->status.burn_state = 0;
        pwork->status.device_burning = 0;
        window_limits();
        this->recvTimer->stop();
    }
}

void MainWindow::start_burning()
{
    if (0 == pwork->status.device_burning && 1 == pwork->status.burn_state)
    {
        pwork->status.device_burning = 1;
        serialBuffer.clear();
        pwork->status.burn_process = 1;
        ATCMDSend("AT+QIOTINFO?\r\n");    //发送AT指令，获取DK
        this->recvTimer->start(300);
    }
}

void MainWindow::ATCMDSend(QString data)
{
    QString dataValue = data + "\r\n";
    if(this->qserial->serialIsOpen())
    {
        this->qserial->SerialSend(dataValue.toUtf8());
    }
    else
    {
        QMessageBox::information(this, tr("警告"), tr("串口打开失败，请检查是否被占用"), tr("确定"));
        pwork->status.burn_state = 0;
        pwork->status.device_burning = 0;
        serialBuffer.clear();
        this->qserial->SerialClose();
        refresh_process_window();
        window_limits();
        this->recvTimer->stop();
        return;
    }
}

void MainWindow::dataReadNoticSlot(QByteArray recvData)
{
    this->recvTimer->stop();
    serialBuffer.append(recvData);
    //新建定时器200ms
    this->recvTimer->start(200);
}

void MainWindow::recvCheckTimeOut()
{
    this->recvTimer->stop();
    if (pwork->status.device_burning == 0)
    {
        return;
    }

    if (0 == pwork->status.burn_state)
    {
        pwork->status.burn_state = 0;
        pwork->status.device_burning = 0;
        serialBuffer.clear();
        this->qserial->SerialClose();
        window_limits();
        return;
    }

    if(!serialBuffer.isEmpty())
    {
        if (pwork->d_info.device_key != strings_handle())
        {
            qDebug()<<"AT返回消息："<<serialBuffer;
        }
        if (pwork->status.work_status == 0)
        {
            if (offline_mode_burning())
            {
                if (pwork->status.burn_process == 2)
                {
                    emit startBurningSignal();
                }
            }
        }
        else if (pwork->status.work_status == 1)
        {
            if (online_mode_burning())
            {
                if (pwork->status.burn_process == 2)
                {
                    emit startBurningSignal();
                }
            }
        }
    }
    else
    {
        if (pwork->status.burn_process == 1)
        {
            Sleep(1000);
            ATCMDSend("AT+QIOTINFO?\r\n");
            this->recvTimer->start(300);
            refresh_process_window();
            ui->label_1_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
            ui->label_1_2->setText(tr("读取DeviceKey -- 读取中..."));
        }
        else if (pwork->status.burn_process == 2)
        {
            ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_3_1->setText("×");
            ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("错误"), tr("写入DeviceSecret失败！"), tr("确定"));
            refresh_process_window();
            window_limits();
        }
    }
}

bool MainWindow::offline_mode_burning()
{
    if (0 == pwork->status.burn_state)
    {
        pwork->status.burn_state = 0;
        pwork->status.device_burning = 0;
        serialBuffer.clear();
        this->qserial->SerialClose();
        window_limits();
        this->recvTimer->stop();
        return false;
    }

    if (pwork->status.burn_process == 1)
    {
        refresh_process_window();
        QString readtext = strings_handle();
        if (readtext == NULL)
        {
            ui->label_1_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_1_1->setText("×");
            ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_1_2->setText(QString(tr("获取DeviceKey -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("错误"), tr("获取DeviceKey失败！"), tr("确定"));
            window_limits();
            refresh_process_window();
            return false;
        }

        if (pwork->d_info.device_key == readtext)
        {
            Sleep(1000);
            ATCMDSend("AT+QIOTINFO?\r\n");
            this->recvTimer->start(300);
            serialBuffer.clear();
            success_process_window();
//            qDebug()<<"当前DK与上一台烧录设备一致";
            return false;
        }
        pwork->d_info.device_key = readtext;

        ui->label_1_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
        ui->label_1_1->setText("√");
        ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
        ui->label_1_2->setText(QString(tr("读取DeviceKey -- 成功：%1").arg(pwork->d_info.device_key)));
        ui->label_1_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");
        ui->label_2_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
        ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:15px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
        ui->label_2_2->setText(tr("获取DeviceSecret -- 获取中..."));

        QString ds = g_csvFile->findSqlData(pwork->d_info.device_key);
        if (ds != NULL)
        {
            pwork->d_info.device_secret = ds;
            dstext = compress_ds(pwork->d_info.device_secret);
            ui->label_2_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
            ui->label_2_1->setText("√");
            ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
            ui->label_2_2->setText(QString(tr("获取DeviceSecret -- 成功：%1").arg(dstext)));
            ui->label_2_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");
        }
        else
        {
            ui->label_2_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_2_1->setText("×");
            ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_2_2->setText(QString(tr("获取DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("警告"), tr("获取DeviceSecret失败！"), tr("确定"));
            refresh_process_window();
            window_limits();
            return false;
        }

        serialBuffer.clear();
        QString at_ds = QString("AT+QIOTCFG=\"default_DS\",\"%1\"").arg(pwork->d_info.device_secret);
        ATCMDSend(at_ds + "\r\n");    //写入DS
        this->recvTimer->start(300);
        ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
        ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
        ui->label_3_2->setText(tr("写入DeviceSecret -- 写入中..."));
        pwork->status.burn_process = 2;
        return true;
    }
    else if (pwork->status.burn_process == 2)
    {
        QString readtext = serialBuffer;
        if (readtext.indexOf("OK") >= 0 && readtext.indexOf(pwork->d_info.device_secret) >= 0)
        {
            ui->label_3_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
            ui->label_3_1->setText("√");
            ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
            ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 成功：烧录完成")));

            pwork->status.burn_state = 1;
            pwork->status.device_burning = 0;
            qDebug()<<"设备烧录成功：DK ="<<pwork->d_info.device_key<<"DS ="<<pwork->d_info.device_secret;
            return true;
        }
        else
        {
            ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_3_1->setText("×");
            ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("错误"), tr("写入DeviceSecret失败！"), tr("确定"));
            refresh_process_window();
            window_limits();
            return false;
        }
    }
    serialBuffer.clear();
    return false;
}

bool MainWindow::online_mode_burning()
{
    if (0 == pwork->status.burn_state)
    {
        pwork->status.burn_state = 0;
        pwork->status.device_burning = 0;
        serialBuffer.clear();
        this->qserial->SerialClose();
        window_limits();
        this->recvTimer->stop();
        return false;
    }

    if (pwork->status.burn_process == 1)
    {
        refresh_process_window();
        QString readtext = strings_handle();
        if (readtext == NULL)
        {
            ui->label_1_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_1_1->setText("×");
            ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_1_2->setText(QString(tr("获取DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("错误"), tr("获取DeviceSecret失败！"), tr("确定"));
            window_limits();
            refresh_process_window();
            return false;
        }

        if (pwork->d_info.device_key == readtext)
        {
            Sleep(1000);
            ATCMDSend("AT+QIOTINFO?\r\n");
            this->recvTimer->start(300);
            serialBuffer.clear();
            success_process_window();
//            qDebug()<<"当前DK与上一台烧录设备一致";
            return false;
        }
        pwork->d_info.device_key = readtext;

        ui->label_1_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
        ui->label_1_1->setText("√");
        ui->label_1_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
        ui->label_1_2->setText(QString(tr("读取DeviceKey -- 成功：%1").arg(pwork->d_info.device_key)));
        ui->label_1_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");
        ui->label_2_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
        ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
        ui->label_2_2->setText(tr("获取DeviceSecret -- 获取中..."));

        QString ds = httpclient->get_device_secret(pwork->d_info.burn_code, pwork->d_info.device_key, cloud);
        if (ds != NULL)
        {
            pwork->d_info.device_secret = ds;
            dstext = compress_ds(pwork->d_info.device_secret);
            ui->label_2_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
            ui->label_2_1->setText("√");
            ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
            ui->label_2_2->setText(QString(tr("获取DeviceSecret -- 成功：%1").arg(dstext)));
            ui->label_2_1_1->setStyleSheet("font-size:23px;color:rgb(0, 170, 0);font-weight:bold;border-color:rgb(255, 255, 255);");
        }
        else
        {
            ui->label_2_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_2_1->setText("×");
            ui->label_2_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_2_2->setText(QString(tr("获取DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            if (pwork->status.network_status == 0)
            {
                QMessageBox::warning(this, tr("错误"), tr("当前网络不可用，已停止烧录"), tr("确认"));
            }
            else
            {
                QMessageBox::warning(this, tr("错误"), tr("获取DeviceSecret失败！"), tr("确定"));
            }
            refresh_process_window();
            window_limits();
            return false;
        }

        serialBuffer.clear();
        QString at_ds = QString("AT+QIOTCFG=\"default_DS\",\"%1\"").arg(pwork->d_info.device_secret);
        ATCMDSend(at_ds + "\r\n");    //写入DS
        this->recvTimer->start(300);
        ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(85, 0, 255);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
        ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(194, 194, 194); color:rgb(85, 0, 255);");
        ui->label_3_2->setText(tr("写入DeviceSecret -- 写入中..."));
        pwork->status.burn_process = 2;
        return true;
    }
    else if (pwork->status.burn_process == 2)
    {
        QString readtext = serialBuffer;
        if (readtext.indexOf("OK") >= 0 && readtext.indexOf(pwork->d_info.device_secret) >= 0)
        {
            ui->label_3_1->setStyleSheet("background-color:rgb(220, 255, 229);color:rgb(0, 170, 0);font-weight:bold;font-size:15px;border-color:rgb(220, 255, 229);border-radius:15px;");
            ui->label_3_1->setText("√");
            ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(0, 170, 0);font-weight:bold;");
            ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 成功：烧录完成")));

            pwork->status.burn_state = 1;
            pwork->status.device_burning = 0;
            qDebug()<<"设备烧录成功：DK ="<<pwork->d_info.device_key<<"DS ="<<pwork->d_info.device_secret;
            this->recvTimer->stop();
            return true;
        }
        else
        {
            ui->label_3_1->setStyleSheet("background-color:rgb(240, 240, 240);color:rgb(207, 0, 0);font-weight:bold;font-size:15px;border-color:rgb(240, 240, 240);border-radius:15px;");
            ui->label_3_1->setText("×");
            ui->label_3_2->setStyleSheet("border-color:rgb(255, 255, 255);font-size:20px;color:rgb(207, 0, 0);font-weight:bold;");
            ui->label_3_2->setText(QString(tr("写入DeviceSecret -- 失败！")));
            pwork->status.burn_state = 0;
            pwork->status.device_burning = 0;
            serialBuffer.clear();
            this->qserial->SerialClose();
            this->recvTimer->stop();
            QMessageBox::warning(this, tr("错误"), tr("写入DeviceSecret失败！"), tr("确定"));
            refresh_process_window();
            window_limits();
            return false;
        }
    }
    serialBuffer.clear();
    return false;
}

void MainWindow::on_pushButton_input_clicked()
{
    if (pwork->status.work_status == 1)
    {
        if (pwork->status.network_status == 0)
        {
//            QMessageBox::information(this, "错误", "当前网络不可用", "确定");
            return;
        }

        inputwindow = new InputWindow(cloud, this);
        connect(inputwindow, SIGNAL(inputResultSignal(QString, QString, QString)), this, SLOT(process_inputwindow_signal(QString, QString, QString)));
        inputwindow->show();
        return;
    }
    else if (pwork->status.work_status == 0)
    {
        QString pktext = g_csvFile->openCsvFile("csv");
        if (pktext != NULL)
        {
            ui->label_pn->setText("---");
            ui->label_pk->setText(pktext);
            ui->pushButton_burn->setDisabled(false);
            ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:15px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
            
            pwork->d_info.file_name = g_csvFile->findSqlData("burnfile");
            if (pwork->d_info.file_name != NULL)
            {
                ui->label_status->setText(pwork->d_info.file_name);
                ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(0, 170, 0);");
            }
            else
            {
                ui->label_status->setText(tr("请导入密钥文件"));
                ui->label_status->setStyleSheet("border-color:rgb(255, 255, 255);font-weight:bold;font-size:13px;color:rgb(255, 0, 0);");
            }
            return;
        }
    }
}

void MainWindow::process_inputwindow_signal(QString code, QString product_name, QString product_key)
{
    pwork->d_info.burn_code = code;
    g_csvFile->setWorkSqlDate("burncode", pwork->d_info.burn_code);
    pwork->d_info.product_name = product_name;
    pwork->d_info.product_key = product_key;
    ui->label_pn->setText(pwork->d_info.product_name);
    ui->label_pk->setText(pwork->d_info.product_key);
    ui->pushButton_burn->setDisabled(false);
    ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:15px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
}

void MainWindow::refresh_info(QString code)
{
    if (code == NULL)
    {
        return;
    }

    QString product_name = "";
    QString product_key = "";
    QString product_info = httpclient->get_product_information(code, cloud);

    QJsonDocument tojson;
    QJsonParseError ParseError;
    tojson =QJsonDocument::fromJson(product_info.toUtf8(), &ParseError);
    if(!tojson.isNull() && ParseError.error == QJsonParseError::NoError)
    {
        if(tojson.isObject())
        {
            QJsonObject Object = tojson.object();
            QJsonObject::const_iterator it = Object.constBegin();
            QJsonObject::const_iterator end = Object.constEnd();
            while(it != end)
            {
                if (it.key() == "pk")
                {
                    product_key = it.value().toVariant().toString();
                }
                if (it.key() == "productName")
                {
                    product_name = it.value().toVariant().toString();
                }
                it++;
            }
        }
    }

    if (product_name == NULL || product_key == NULL)
    {
        qDebug()<<"获取产品信息失败";
//        QMessageBox::information(this, "提示", "获取产品信息失败", "确认");
        return;
    }

    qDebug()<<"product_name ="<<product_name;
    qDebug()<<"product_key ="<<product_key;

    pwork->d_info.product_name = product_name;
    pwork->d_info.product_key = product_key;
//    ui->label_pn->setText(pwork->d_info.product_name);
//    ui->label_pk->setText(pwork->d_info.product_key);
//    ui->pushButton_burn->setDisabled(false);
//    ui->pushButton_burn->setStyleSheet("background-color:rgb(22,155,213);font-weight:bold;font-size:15px;color:rgb(255, 255, 255);border-width:2px;border-radius:7px;");
}

void MainWindow::network_drops(int network_status)
{
//    qDebug()<<"当前网络状态为："<<network_status;
    pwork->status.network_status = network_status;
    if (pwork->status.work_status == 1)
    {
        if (pwork->status.network_status == 0)
        {
            if (pwork->status.work_status == 1)
            {
                if (pwork->status.device_burning == 1)
                {
                    pwork->status.device_burning = 0;
                    pwork->status.burn_state = 0;
                    serialBuffer.clear();
                    this->qserial->SerialClose();
                    this->recvTimer->stop();
                    QMessageBox::warning(this, tr("错误"), tr("当前网络不可用，已停止烧录"), tr("确认"));
                    refresh_process_window();
                    window_limits();
                }
            }
            refresh_window();
            inputwindow->close_input_window();
        }
        else if (pwork->status.network_status == 1)
        {
            refresh_window();
        }
    }
}

QString MainWindow::compress_ds(QString ds)
{
    QByteArray byteArray = ds.toLatin1();
    const char *s = byteArray.data();
    char ret1[4];
    char ret2[4];
    for (int i = 0; i < 32; i++)
    {
        if ((i < 4))
        {
            ret1[i] = *s;
        }
        else if ((i >= 28))
        {
            ret2[i - 28] = *s;
        }
        s ++;
    }

    QString results = QString::fromLocal8Bit(ret1,4) + "****" + QString::fromLocal8Bit(ret2,4);
    qDebug()<<"device_secret = "<<results;
    return results;
}

QString MainWindow::strings_handle()
{
    QString dkstring = NULL;
    char *str = strstr(serialBuffer, "\"uuid\"");
    if (!str)
    {
        qDebug()<<"获取DK失败！"<<dkstring;
        return dkstring;
    }
    QStringList list = QString::fromUtf8(str).split('"');
    dkstring = list[3];
//    qDebug()<<"获取DK成功："<<dkstring;
    return dkstring;
}

void MainWindow::on_hideButton_clicked()
{
    if (ui->hideButton->text() == "︽")
    {
        ui->hideButton->setText("︾");
        ui->label_4->setVisible(false);
        ui->comboBox_stopBits->setVisible(false);
        ui->label_6->setVisible(false);
        ui->comboBox_parity->setVisible(false);
    }
    else if (ui->hideButton->text() == "︾")
    {
        ui->hideButton->setText("︽");
        ui->label_4->setVisible(true);
        ui->comboBox_stopBits->setVisible(true);
        ui->label_6->setVisible(true);
        ui->comboBox_parity->setVisible(true);
    }
}

void MainWindow::on_action_update_triggered()
{
    if (pwork->status.network_status == 1)
    {
        updateVersion->checkForUpdates();
    }
    else if (pwork->status.network_status == 0)
    {
        QMessageBox::warning(this, tr("错误"), tr("当前网络不可用"), tr("确定"));
    }
}

void MainWindow::on_action_about_triggered()
{
    updateVersion->softwareInformation();
}

void MainWindow::on_action_doc_triggered()
{
    QString oldFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-Auth-Burner-Win/useDoc.chm";
    QFileInfo oldFileInfo(oldFile);
    QString newFile = ":/doc/useDoc.chm";
    QFileInfo newFileInfo(newFile);
    QDateTime oldFileTime = oldFileInfo.lastModified();//最后修改时间
    QDateTime newFileTime = newFileInfo.lastModified();//最后修改时间
    if(false == oldFileInfo.isFile() || newFileTime > oldFileTime)
    {
        QFile::remove(oldFile);
        QFile::copy(newFile, oldFile);
        qDebug()<<"需要拷贝文件";
    }
    QProcess *helpProcess = new QProcess(this);
    QStringList argument(oldFile);
    helpProcess->start("hh.exe",argument);
}

void MainWindow::languageChange(int mode)
{
    if(NULL != main_translator)
    {
        qApp->removeTranslator(main_translator);
        delete main_translator;
        main_translator = NULL;
    }

    QString hideButtonText = ui->hideButton->text();
    main_translator = new QTranslator(this);
    if(0 == mode)
    {
        main_translator->load(":/chinese.qm");
        qApp->installTranslator(main_translator);
        g_csvFile->setWorkSqlDate("language", "chinese");
    }
    else if(1 == mode)
    {
        main_translator->load(":/english.qm");
        qApp->installTranslator(main_translator);
        g_csvFile->setWorkSqlDate("language", "english");
    }

    ui->retranslateUi(this);
    ui->hideButton->setText(hideButtonText);
    refresh_window();
}

void MainWindow::on_action_chinese_triggered()
{
    languageChange(0);
}

void MainWindow::on_action_english_triggered()
{
    languageChange(1);
}

void MainWindow::on_action_CN_triggered()
{
    ui->label_cloud->setText(tr("中国数据中心"));
    cloud = 0;
    g_csvFile->setWorkSqlDate("cloud", QString::number(cloud));
    refresh_window();
}

void MainWindow::on_action_EU_triggered()
{
    ui->label_cloud->setText(tr("欧洲数据中心"));
    cloud = 1;
    g_csvFile->setWorkSqlDate("cloud", QString::number(cloud));
    refresh_window();
}

void MainWindow::on_action_US_triggered()
{
    ui->label_cloud->setText(tr("美国数据中心"));
    cloud = 2;
    g_csvFile->setWorkSqlDate("cloud", QString::number(cloud));
    refresh_window();
}

void MainWindow::on_action_dumpLog_triggered()
{
    openlog->cpDateDump();
}

void MainWindow::on_action_userLog_triggered()
{
    openlog->cpDateLog();
}
