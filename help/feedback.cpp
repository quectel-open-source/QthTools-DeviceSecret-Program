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
#include "feedback.h"
#include "ui_feedback.h"

feedback::feedback(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::feedback)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
}

feedback::~feedback()
{
    delete ui;
}