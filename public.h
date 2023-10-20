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
#ifndef PUBLIC_H
#define PUBLIC_H

#include <QString>

class PublicWork
{
public:
    struct workStatus{
        int work_status = 0;    //工作模式，1在线模式，0离线模式
        int network_status = 1;    //网络状态，1有连接，0无连接
        int burn_state = 0;    //烧录状态，1开始烧录，0停止烧录
        int device_burning = 0;    //设备烧录状态，1正在烧录，0烧录结束
        int burn_process = 1;    //烧录流程，1获取DK，写入DS
    };
    struct workInfo{
        QString burn_code = "";
        QString product_name = "";
        QString product_key = "";
        QString device_key = "";
        QString device_secret = "";
        QString file_name = "";
    };

    workStatus status;
    workInfo d_info;
};


//struct workStatus status;
//struct workInfo d_info;

#endif // PUBLIC_H
