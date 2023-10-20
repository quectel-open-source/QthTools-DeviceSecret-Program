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
#include "mysqlite.h"

#include <QStandardPaths>
#include <QDebug>
#include<QDir>

mySqlite::mySqlite()
{

}

mySqlite::~mySqlite()
{

}

void mySqlite::createSqlDataBase(QString path)
{
    // 指定的连接名称
    QString dstPath = path;
    if (QSqlDatabase::contains("sqlite3"))
    {
        database = QSqlDatabase::database("sqlite3");
    }
    else
    {
        qDebug()<<"dstPath:"<<dstPath;
        // 添加数据库
        database = QSqlDatabase::addDatabase("QSQLITE","sqlite3");
        // 数据库文件名:如果这个数据库不存在，则会在后续操作时自动创建；如果已经存在，则后续的操作会在已有的数据库上进行
        database.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/QthTools-Auth-Burner-Win/" + dstPath + ".db");
        database.setUserName("quectel");  //设置数据库登录用户名
        database.setPassword("123456");//设置数据库登录密
    }
}
/**************************************************************************
** 功能	@brief : 更新sqltable数据
** 输入	@param :
** 输出	@retval:
** 注意  @note: 存在表则切换，不存在则创建
**      @example:
***************************************************************************/
void mySqlite::createSqlQuery(QString tableName, QString data)
{
    // 打开数据库
    if(!database.open())
    {
        qCritical()<<"open error";
        return;
    }
    QStringList tableNameList = database.tables();
    bool isFind = false;
    foreach (QString name, tableNameList)
    {
        // sqlite_sequence 排除
        if ( QString("sqlite_sequence") == name)
        {
            continue;
        }
        if(0 == tableName.compare(name))
        {
            isFind = true;
            break;
        }
    }
    if(false == isFind)
    {
        query = new QSqlQuery(database);
        if(false == query->prepare(data))
        {
            qDebug()<<"error table"<<query->lastError();
        }
        if(query->exec(data))
        {
            qDebug()<<"query->lastError():"<<query->lastError();
        }
    }
    else
    {
        query = new QSqlQuery(database);
        if(query->exec(data))
        {
            qDebug()<<"query->lastError():"<<query->lastError();
        }
    }
}
/**************************************************************************
** 功能	@brief : 插入sqltable数据
** 输入	@param :
** 输出	@retval:
** 注意  @note:
**      @example: insert into student value(3, \"Li\", 23)
***************************************************************************/
bool mySqlite::insertSqlData(QString data,int *errorColumn)
{
    if(!database.isOpen())
    {
        return false;
    }
    if(!query->exec(data))
    {
        qDebug() <<"insert error:" <<query->lastError();
        if(query->lastError().text().contains("device_userInfo.DeviceKey"))
        {
            *errorColumn = 0;
        }
        return false;
    }
    return true;
}

int mySqlite::batchInsertSqlData(QList<structInfo_t> data)
{
    if(!database.isOpen())
    {
        return -1;
    }   
    int sum = data.size();
    if (sum > 0)
    {
        database.transaction();
        for(int i = 0; i < sum; i ++)
        {
            if(!query->exec(QString("insert into device_userInfo(ProductKey, ProductSecret, DeviceKey, DeviceSecret) values('%1', '%2', '%3', '%4')").arg(data[i].ProductKey).arg(data[i].ProductSecret).arg(data[i].DeviceKey).arg(data[i].DeviceSecret)))
            {
                qDebug() <<"insert error:" <<query->lastError();
                database.rollback();
                return i;
            }
        }
        database.commit();
    }
    return 0;
}

int mySqlite::batchInsertSqlData(QList<structWork_t> data)
{
    if(!database.isOpen())
    {
        return -1;
    }
    int sum = data.size();
    if (sum > 0)
    {
        database.transaction();
        for(int i = 0; i < sum; i ++)
        {
            if(!query->exec(QString("insert into device_workInfo(name, status) values('%1', '%2')").arg(data[i].name).arg(data[i].status)))
            {
                qDebug() <<"insert error:" <<query->lastError();
                database.rollback();
                return i;
            }
        }
        database.commit();
    }
    return 0;
}

void mySqlite::modifyWorkSqlDate(QString data)
{
    if(!database.isOpen())
    {
        return;
    }

    if(!query->exec(data))
    {
        qDebug() <<"modify error:" <<query->lastError();
        database.rollback();
        return;
    }
}
/**************************************************************************
** 功能	@brief : 查找sqltable数据
** 输入	@param :
** 输出	@retval:
** 注意  @note:
**      @example: select id, name from student"; select * from student select max(id) from student
***************************************************************************/
QStringList mySqlite::findSqlDataList(QString data)
{
    QStringList result;
    if(!database.isOpen())
    {
        return result;
    }
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        return result;
    }
    QSqlRecord rec = query->record();
    int cols = rec.count();

    while(query->next())
    {
        QString info;
        for(int i =0;i<cols;i++)
        {
            info.append(query->value(i).toString()+',');
        }
        result.append(info);
    }
    return result;
}


QStringList mySqlite::execSqlDataList(QString data)
{
    QStringList result;
    if(!database.isOpen())
    {
        goto exit;
    }
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        goto exit;
    }
    while(query->next())
    {
        result.append(query->value(0).toString());
    }
exit:
    return result;
}



QString mySqlite::findSqlDataString(QString data)
{
    QString result;
    if(!database.isOpen())
    {
        goto exit;
    }
        // 打开数据库
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        goto exit;
    }
    while(query->next())
    {
        result.append(query->value(0).toString());
    }
exit:
    return result;
}

QVariant mySqlite::findSqlData(QString data, int mode)
{

    QString result = "unKnow";
    if(!database.isOpen())
    {
        goto exit;
    }
    if(mode == 2)
    {
        QStringList result;
        if(!query->exec(data))
        {
            qDebug() << query->lastError();
            goto exit;
        }
        if(query->next())
        {
            QSqlRecord rec = query->record();
            for(int i = 0;i<rec.count();i++)
                result.append(query->value(i).toString());
        }
        goto exit;
    }
    else if(mode == 1)
    {
        int result = 0;
        if(!query->exec(data))
        {
            qDebug() << query->lastError();
            return (QVariant)result;
        }
        return (QVariant)(query->size());
    }
exit:
    return QVariant(result);
}

QVector<structInfo_t> mySqlite::findSqlAllList(QString data)
{
    QVector<structInfo_t> it;
    if(!database.isOpen())
    {
        goto exit;
    }
    // 打开数据库
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        return it;
    }
    while(query->next())
    {
        structInfo_t info;
        info.DeviceKey = query->value(1).toString();
        info.DeviceSecret = query->value(2).toString();
        it.append(info);
    }
exit:
    return it;
}


/**************************************************************************
** 功能	@brief : 删除sqltable数据
** 输入	@param :
** 输出	@retval:
** 注意  @note:
**      @example: delete from student where id = ?"; delete from student  delete from student where id=1 and name='xiaopi'
***************************************************************************/
bool mySqlite::delSqlData(QString data)
{
    if(!database.isOpen())
    {
        return false;
    }
    // 打开数据库
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        return false;
    }
    return true;
}



int mySqlite::batchDelSqlData(QList<QString> data)
{
    if(!database.isOpen())
    {
        return -1;
    }
    int sum = data.size();
    if(sum > 0)
    {
        database.transaction();
        for(int i=0;i<sum;i++)
        {
            if(!query->exec(QString("delete from device_userInfo where DeviceKey='%1';").arg(QString(data[i]))))
            {
                qDebug() <<"insert error:" <<query->lastError();
                database.rollback();
                return i;
            }
        }
        database.commit();
    }
    return 0;
}


/**************************************************************************
** 功能	@brief : 更新sqltable数据
** 输入	@param :
** 输出	@retval:
** 注意  @note:
**      @example: update student set name='小皮' where id=1
***************************************************************************/
bool mySqlite::modifySqlData(QString data)
{
#ifdef ERROR_LEVEL
    qDebug()<<__FUNCTION__;
#endif
    // 打开数据库
    if(!database.isOpen())
    {
        return false;
    }
    if(!query->exec(data))
    {
        qDebug() << query->lastError();
        return false;
    }

    return true;
}

/**************************************************************************
** 功能	@brief : 关闭sql数据库
** 输入	@param :
** 输出	@retval:
** 注意  @note:
**      @example:
***************************************************************************/
void mySqlite::closeSqlDataBase()
{
    // 打开数据库
    if(!database.isOpen())
    {
        qCritical()<<"open error";
        return;
    }
    database.close();
}
