/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 * Origin copyright bearer: 2019 - 2026 UnionTech Software Technology Co., Ltd.
 * Ported from DDE calendar tag 6.6.0 (upstream license: LGPL-3.0-or-later,
 * relicensed under GPL-3.0-or-later as permitted by LGPL-3.0 section 3).
 * Trimmed: dropped the account/sync/CalDAV/remind/upload schemas and the multi-database transaction locker; this port keeps a single SQLite file.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-service/src/dbmanager/ddatabase.h）。
 */
#ifndef DDATABASE_H
#define DDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QMap>
#include <QStringList>

class DDataBase : public QObject
{
    Q_OBJECT
public:
    //日程表
    static const QString sql_create_schedules;
    //类型表
    static const QString sql_create_scheduleType;
    //颜色表
    static const QString sql_create_typeColor;

    //工作颜色id
    static const QString GWorkColorID;
    //生活颜色id
    static const QString GLifeColorID;
    //其他颜色id
    static const QString GOtherColorID;
    //节假日颜色id
    static const QString GFestivalColorID;

public:
    explicit DDataBase(QObject *parent = nullptr);
    virtual ~DDataBase();

    QString getDBPath() const;
    void setDBPath(const QString &DBPath);
    static QString createUuid();

    QString getConnectionName() const;
    void setConnectionName(const QString &connectionName);

    //初始化数据库数据，会创建数据库文件和相关数据表
    virtual void initDBData();
    void dbOpen();

    //判断数据库是否存在
    bool dbFileExists();

    //删除db数据库文件
    bool removeDB();

protected:
    //创建数据库
    virtual void createDB() = 0;

protected:
    QSqlDatabase m_database;
    QString m_DBPath;
    QString m_connectionName;
};

/**
 * @brief The DbPathMutex struct 整理了sqlite数据库文件锁的相关操作
 */
struct SqliteMutex {
private:
    /**
     * @brief The SqliteMutex struct 用于跳过QMutex的拷贝构造函数和operator=函数的调用
     */
    struct UnCopyMutex {
        UnCopyMutex(){}
        UnCopyMutex(const UnCopyMutex &) {}
        UnCopyMutex &operator=(const UnCopyMutex &) {return *this;}
        void lock();
        void unlock();

    private:
        QMutex m;
    } m;                                //数据库文件锁
    bool transactionLocked = false;     //是否开启事务 及 数据库文件是否被锁定
    qint64 transactionThreadId = 0;     //开启事务的线程id

public:
    /**
     * @brief lock 数据库文件被锁定，用于非事务场景
     */
    void lock();

    /**
     * @brief unlock 数据库文件被解锁，用于非事务场景
     */
    void unlock();

    /**
     * @brief transactionLock 数据库文件被锁定，用非事务场景
     */
    void transactionLock();

    /**
     * @brief transactionUnlock 数据库文件被解锁，用非事务场景
     */
    void transactionUnlock();
};

/**
 * @brief The SqliteQuery class 根据数据库文件锁来执行sql语句的类
 */
class SqliteQuery : public QSqlQuery {
public:
    explicit SqliteQuery(QSqlDatabase db);
    explicit SqliteQuery(const QString &connectionName);
    SqliteQuery(const QString &query, QSqlDatabase db);

    bool exec(QString sql);
    bool exec();

    bool transaction();
    bool commit();
    void rollback();

private:
    QSqlDatabase _db;
};

// 参考实现里的 SqlTransactionLocker 用于跨多个库文件（账户管理库 + 各账户库）
// 协调事务，并明确说明 SQLite 无法跨连接保证原子性。
// 本项目只有一个库文件、一个连接，这种协调没有意义，所以不保留；
// 需要事务时直接用 SqliteQuery::transaction()/commit()/rollback()。

#endif // DDATABASE_H
