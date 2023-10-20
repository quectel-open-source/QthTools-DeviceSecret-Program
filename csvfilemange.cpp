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
#include "csvfilemange.h"
#include "mysqlite.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include<QCoreApplication>

QObject *userInfoSql = NULL;

csvFileMange::csvFileMange()
{

}

void csvFileMange::openSqlDataBase()
{
    // 新建/打开 数据库
    userInfoSql = new mySqlite();
    ((mySqlite *)userInfoSql)->createSqlDataBase("device_userInfo");
    // 新建/打开表
    createSqlTable();
}

/**************************************************************************
** 功能	@brief : 清空用户数据
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void csvFileMange::clearData()
{
    // 删除所有数据  // 清空多少个stackWidget 如何清空数据库
    if(NULL != userInfoSql)
    {
        QString delInfo = QString("delete from device_userInfo;");
        ((mySqlite *)userInfoSql)->delSqlData(delInfo);
        QString delInfoSeq = QString("DELETE FROM sqlite_sequence WHERE name = 'device_userInfo';");
        ((mySqlite *)userInfoSql)->delSqlData(delInfoSeq);
    }
}

void csvFileMange::createSqlTable()
{
//    qDebug()<<__FUNCTION__;
    qDebug()<<"创建数据库";
    if (NULL != userInfoSql)
    {
        ((mySqlite *)userInfoSql)->createSqlQuery("device_userInfo", \
            "CREATE TABLE device_userInfo (idex integer UNIQUE,"
            "ProductKey TEXT(10),"
            "ProductSecret TEXT(20),"
            "DeviceKey TEXT(40),"
            "DeviceSecret TEXT(40),"
            "PRIMARY KEY(idex AUTOINCREMENT));");
    }
    if (NULL != userInfoSql)
    {
        ((mySqlite *)userInfoSql)->createSqlQuery("device_userInfo", \
            "CREATE TABLE device_workInfo (idex integer UNIQUE,"
            "name TEXT(20),"
            "status TEXT(50),"
            "PRIMARY KEY(idex AUTOINCREMENT));");

        QStringList list;
        bool Workingmode = false;
        bool Burncode = false;
        bool Burnfile = false;
        bool Language = false;
        bool Cloud = false;
        list = ((mySqlite *)userInfoSql)->findSqlDataList("SELECT name, status FROM device_workInfo;");
        if (list.size() > 0)
        {
            if (QString(list.at(0)).contains(','))
            {
                for (int i = 0; i < list.size(); i ++)
                {
                    if (QString(list.at(i)).split(",").at(0) == "workingmode")
                    {
                        Workingmode = true;
                    }
                    else if (QString(list.at(i)).split(",").at(0) == "burncode")
                    {
                        Burncode = true;
                    }
                    else if (QString(list.at(i)).split(",").at(0) == "burnfile")
                    {
                        Burnfile = true;
                    }
                    else if (QString(list.at(i)).split(",").at(0) == "language")
                    {
                        Language = true;
                    }
                    else if (QString(list.at(i)).split(",").at(0) == "cloud")
                    {
                        Cloud = true;
                    }
                }
            }
        }

        QList<structWork_t>infoList;
        structWork_t winfo;
        if (!Workingmode)
        {
            winfo.name = "workingmode";
            winfo.status = "0";
            infoList.append(winfo);
        }
        if (!Burncode)
        {
            winfo.name = "burncode";
            winfo.status = "";
            infoList.append(winfo);
        }
        if (!Burnfile)
        {
            winfo.name = "burnfile";
            winfo.status = "";
            infoList.append(winfo);
        }
        if (!Language)
        {
            winfo.name = "language";
            winfo.status = "";
            infoList.append(winfo);
        }
        if (!Cloud)
        {
            winfo.name = "cloud";
            winfo.status = "";
            infoList.append(winfo);
        }
        ((mySqlite *)userInfoSql)->batchInsertSqlData(infoList);
        infoList.clear();
    }
}

void csvFileMange::nonBlockingSleep(int sectime)
{
    QTime dieTime = QTime::currentTime().addMSecs(sectime);
    int g_maxTime = 1;
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, g_maxTime);
    }
}

bool csvFileMange::fileVaildCheck(QTextStream *in, int fileSize, int *lineCount)
{
    int currentFileOffset = 0;
    // 数据有效性检测
    bool fileValid = true;
    bool dataValid = true;
    bool duplicatedata = true;
    int lineCountsCheck = 0;
    int oldPercent = 0;
    nonBlockingSleep(4);
    QString fileData = in->readLine();
    QStringList list = fileData.split(',');

    if (list.size() != 4)
    {
        QMessageBox::information(0, QObject::tr("错误"), QObject::tr("文件标题出错，请修正后重新导入"), QObject::tr("确认"));
        return false;
    }

    if (fileData != "\"ProductKey\",\"ProductSecret\",\"DeviceKey\",\"DeviceSecret\"")
    {
        QMessageBox::information(0, QObject::tr("错误"), QObject::tr("文件内容错误，请修正后重新导入"), QObject::tr("确认"));
        return false;
    }

    QString addDK = NULL;
    while (in->readLineInto(&fileData))
    {
        if (!fileData.isEmpty() && fileData.size() > 4)
        {
            QStringList list = fileData.split(',');
            if (list.size() != 4)
            {
                fileValid = false;
                break;
            }

            QString pkString = list[0];
            QString psString = list[1];
            QString dkString = list[2];
            QString dsString = list[3];

            if (addDK.indexOf(dkString) == -1)
            {
                addDK = addDK + dkString;
            }
            else
            {
                duplicatedata = false;
                break;
            }

            if ((dsString.size() == 34))
            {
                QByteArray byteArray = dsString.toLatin1();
                const char *s = byteArray.data();
                while (*s)
                {
                    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s == '"'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s ++;
                }
            }
            else
            {
                dataValid = false;
                break;
            }

            if ((pkString.size() == 8))
            {
                QByteArray byteArray = pkString.toLatin1();
                const char *s = byteArray.data();
                while (*s)
                {
                    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s == '"'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s ++;
                }
            }
            else
            {
                dataValid = false;
                break;
            }

            if ((psString.size() == 18))
            {
                QByteArray byteArray = psString.toLatin1();
                const char *s = byteArray.data();
                while (*s)
                {
                    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s == '"'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s ++;
                }
            }
            else
            {
                dataValid = false;
                break;
            }

            if ((psString.size() >= 6) && (psString.size() <= 34))
            {
                QByteArray byteArray = dkString.toLatin1();
                const char *s = byteArray.data();
                while (*s)
                {
                    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s == '"'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s ++;
                }
            }
            else
            {
                dataValid = false;
                break;
            }

            lineCountsCheck ++;
            currentFileOffset+=fileData.size();
            int percent = (currentFileOffset * 1.0 / fileSize) * 100;
            if (oldPercent != percent)
            {
                oldPercent = percent;
                nonBlockingSleep(1);
            }
        }
        else
        {
            dataValid = false;
            break;
        }
    }

    if(false == fileValid || false == dataValid || false == duplicatedata)
    {
        if(false == dataValid)
            QMessageBox::information(0, QObject::tr("错误"), QObject::tr("引入错误文件，请重新导入"), QObject::tr("确认"));
        else if (false == dataValid)
            QMessageBox::information(0, QObject::tr("错误"), QString(QObject::tr("文件第%1行出现错误，请检查后重新导入").arg(lineCountsCheck + 2)), QObject::tr("确认"));
        else if (false == duplicatedata)
            QMessageBox::information(0, QObject::tr("错误"), QString(QObject::tr("文件第%1行\"DeviceKey\"出现重复，请检查后重新导入").arg(lineCountsCheck + 2)), QObject::tr("确认"));
        return false;
    }

    *lineCount = lineCountsCheck;
    nonBlockingSleep(2);
    return true;
}

/**************************************************************************
** 功能	@brief : 打开csv文件
** 输入	@param :
** 输出	@retval:
***************************************************************************/
QString csvFileMange::openCsvFile(QString fileType)
{
//    qDebug()<<__FUNCTION__;
    QString pk = "";
    QString fileName = "";
    if (fileType.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(NULL, QObject::tr("选取文件"), ".", "");
    }
    else
    {
        fileName = QFileDialog::getOpenFileName(NULL, QObject::tr("选取文件"), ".", fileType + " (*."+fileType+")");
    }

    if (fileName.isEmpty())
    {
        return NULL;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite))
    {
        QMessageBox::information(0, QObject::tr("文件管理"), QObject::tr("无法打开文件,请检查文件是否被打开"), QObject::tr("确认"));
        return NULL;
    }

    QFileInfo info(fileName);
    int fileSize = info.size();
    int totalTimes = 6;
    QTextStream in(&file);
    QString g_filePath = QDir::currentPath()+"/";
    QString fileData = "";
    int j= 0;
    QString insertData;
    bool titleFlag = false;
    int lineCounts = 0;

    if(false == fileVaildCheck(&in, fileSize, &lineCounts))
    {
        return NULL;
    }

    if (NULL != userInfoSql)
    {
        clearData();
        createSqlTable();
    }
    else
    {
        qDebug()<<"csv文件打开失败";
        return NULL;
    }

    int oldPercent = 0;
    in.seek(0);
    QList<structInfo_t>infoList;

    for(int i = 0;i < lineCounts + 1; i ++)
    {
        fileData = in.readLine();
        if(!fileData.isEmpty())
        {
            QStringList list = fileData.split('"');
            if(titleFlag)
            {
                structInfo_t info;
                info.ProductKey = list[1];
                info.ProductSecret = list[3];
                info.DeviceKey = list[5];
                info.DeviceSecret = list[7];

                infoList.append(info);
                qDebug()<<list[1]<<list[3]<<list[5]<<list[7];
                pk = list[1];
                j ++;
                if(j >= 0)
                {
                    int line = ((mySqlite *)userInfoSql)->batchInsertSqlData(infoList);
                    infoList.clear();
                    if (line > 0)
                    {
                        QMessageBox::information(0, QObject::tr("错误"), QString(QObject::tr("文件第%1行出现重复错误，请检查后重新导入").arg(line + i - 500 + 2)), QObject::tr("确认"));
                        file.close();
                        return NULL;
                    }
                    j = 0;
                }
            }
            else
            {
                titleFlag = true;
                qDebug()<<"list:"<<list[1]<<list[3]<<list[5]<<list[7];
            }

            if (totalTimes <= 0)
            {
                totalTimes = 6;
                int percent = (i * 1.0 / lineCounts) * 100;
                if(oldPercent != percent)
                {
                    nonBlockingSleep(1);
                }
            }
            else
            {
                totalTimes --;
            }
        }
    }
    file.close();
    if (j != 0)
    {
        int line = ((mySqlite *)userInfoSql)->batchInsertSqlData(infoList);
        infoList.clear();
        if (line > 0)
        {
            QMessageBox::information(0, QObject::tr("错误"), QString(QObject::tr("文件第%1行出现重复错误，请检查后重新导入").arg(lineCounts > 500 ? lineCounts - 500 + line + 2 : line + 2)), QObject::tr("确认"));
            return NULL;
        }
    }

    QStringList list = ((mySqlite *)userInfoSql)->execSqlDataList("SELECT count(ProductKey) AS number FROM device_userInfo;");
    if (QString(list.at(0)).toInt() <= 0)
    {
        QMessageBox::information(0, QObject::tr("警告"), QObject::tr("文件内容为空，请检查文件"), QObject::tr("确认"));
    }

    QStringList list_file = fileName.split('/');
    setWorkSqlDate("burnfile", list_file[list_file.size() - 1]);
    return pk;
}

QString csvFileMange::findSqlData(QString fname)
{
    QString idex = NULL;
    QStringList list;

    if (fname == "workingmode" || fname == "burncode" || fname == "burnfile" || fname == "language" || fname == "cloud")
    {
        list = ((mySqlite *)userInfoSql)->findSqlDataList("SELECT name, status FROM device_workInfo;");
    }
    else
    {
        list = ((mySqlite *)userInfoSql)->findSqlDataList("SELECT DeviceKey, DeviceSecret FROM device_userInfo;");
    }

    if (list.size() > 0)
    {
        if (QString(list.at(0)).contains(','))
        {
            for (int i = 0; i < list.size(); i ++)
            {
                if (QString(list.at(i)).split(",").at(0) == fname)
                {
                    idex = QString(QString(list.at(i)).split(',').at(1));
                    qDebug()<<fname<<"->"<<idex;
                    break;
                }
            }
        }
    }
    return idex;
}

QString csvFileMange::findPkData()
{
    QString idex = NULL;
    QStringList list = ((mySqlite *)userInfoSql)->findSqlDataList("SELECT ProductKey FROM device_userInfo;");

    if (list.size() > 0)
    {
        if (QString(list.at(0)).contains(','))
        {
            for (int i = 0; i < list.size(); i ++)
            {
                idex = QString(QString(list.at(i)).split(',').at(0));
            }
        }
    }
    return idex;
}

void csvFileMange::setWorkSqlDate(QString name, QString info)
{
    ((mySqlite *)userInfoSql)->modifyWorkSqlDate(QString("update device_workInfo set status = '%1' where name = '%2';").arg(info).arg(name));
//    qDebug()<<"burnfile = "<<findSqlData("burnfile");
}
