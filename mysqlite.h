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
#ifndef MYSQLITE_H
#define MYSQLITE_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QtSql/QSqlDriver>    //数据库驱动种类相关头文件
#include <QtSql/QSqlRecord>    //数据库执行结果相关头文件
#include <QtSql/QSqlError>

typedef struct
{
    QString ProductKey;
    QString ProductSecret;
    QString DeviceKey;
    QString DeviceSecret;
}structInfo_t;

typedef struct
{
    QString name;
    QString status;
}structWork_t;

class mySqlite: public QObject
{
    Q_OBJECT
public:
    mySqlite();
    ~mySqlite();
    void createSqlDataBase(QString path);
    void createSqlQuery(QString tableName,QString data);
    void modifyWorkSqlDate(QString data);
    bool insertSqlData(QString data,int *errorColumn);
    int batchInsertSqlData(QList<structInfo_t>data);
    int batchInsertSqlData(QList<structWork_t> data);
    QStringList findSqlDataList(QString data);
    QString findSqlDataString(QString data);
    QVariant findSqlData(QString data,int mode);
    QVector<structInfo_t> findSqlAllList(QString data);
    bool delSqlData(QString data);
    int batchDelSqlData(QList<QString> data);
    bool modifySqlData(QString data);
    QStringList execSqlDataList(QString data);
    void closeSqlDataBase();
    void test();

private:
    QSqlDatabase database;
    QSqlQuery *query;
};

#endif // MYSQLITE_H
