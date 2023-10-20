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
#ifndef CSVFILEMANGE_H
#define CSVFILEMANGE_H
//#include "mysqlite.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QTime>

class csvFileMange
{
public:
    csvFileMange();
    QString openCsvFile(QString fileType);
    void clearData();
    void createSqlTable();
    void setWorkSqlDate(QString name, QString info);
    bool fileVaildCheck(QTextStream *in,int fileSize,int *lineCount);
    void nonBlockingSleep(int sectime);
    void openSqlDataBase();
    QString findSqlData(QString fname);
    QString findPkData();


private:
    //QObject *userInfoSql = NULL;
};

#endif // CSVFILEMANGE_H
