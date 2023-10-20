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
#include "mythead.h"
#include <QDebug>
#include <QProcess>

static int old_network_status = 1;

myThead::myThead()
{

}

void myThead::netlink_work(int mold_network_status)
{
    old_network_status = mold_network_status;
}

void myThead::run()
{
    while (1)
    {
        int network_status;
        QString networkCmd = "ping www.baidu.com -n 2 -w 500";
        QProcess process;
        process.start(networkCmd);
        process.waitForFinished();
        QString result = process.readAll();
        if (result.contains("TTL="))
        {
            network_status = 1;
        }
        else
        {
            network_status = 0;
        }

        if (old_network_status != network_status)
        {
            if (network_status == 0)
            {
                qDebug()<<"当前网络不可用";
            }
            if (network_status == 1)
            {
                qDebug()<<"当前网络已连接";
            }
            old_network_status = network_status;
            emit networkDropsSignal(network_status);
        }
        sleep(3);
    }
}
