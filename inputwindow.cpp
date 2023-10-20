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
#include "inputwindow.h"
#include "ui_inputwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QRegExpValidator>

int inputCloud = 0;

InputWindow::InputWindow(int cloud, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputWindow)
{
    ui->setupUi(this);
    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]+$")));
    ui->lineEdit->setMaxLength(39);
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMinMaxButtonsHint);
    httpclient = new HttpClient();
    status = 1;
    inputCloud = cloud;
}

InputWindow::~InputWindow()
{
    delete ui;
}


void InputWindow::on_pushButton_clicked()
{
    QString burn_code = ui->lineEdit->text();
    if (burn_code.size() != 39)
    {
        QMessageBox::information(this, tr("提示"), tr("烧录码长度不足，请检查烧录码是否正确"), tr("确认"));
        return;
    }

    QString product_name = "";
    QString product_key = "";
    QString product_info = httpclient->get_product_information(burn_code, inputCloud);

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
        QMessageBox::information(this, tr("提示"), tr("获取产品信息失败，请检查烧录码"), tr("确认"));
        return;
    }

    qDebug()<<"burn_code ="<<burn_code;
    qDebug()<<"product_name ="<<product_name;
    qDebug()<<"product_key ="<<product_key;

    emit inputResultSignal(burn_code, product_name, product_key);
    status = 0;
    this->close();
}

void InputWindow::close_input_window()
{
    if (status == 1)
    {
        QMessageBox::warning(this, tr("错误"), tr("当前网络不可用"), tr("确认"));
        this->close();
    }
}

