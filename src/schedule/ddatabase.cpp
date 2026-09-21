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
 * Trimmed to match ddatabase.h.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-service/src/dbmanager/ddatabase.cpp）。
 */
#include "ddatabase.h"
#include "commondef.h"

#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QDebug>

static QMap<QString, SqliteMutex> DbpathMutexMap;//记录所有用到的数据库文件锁
static QMutex DbpathMutexMapMutex;               //DbpathMutexMap的锁

/**
 * @brief getDbMutexRef 根据dbpath获取数据库文件锁的引用
 */
SqliteMutex &getDbMutexRef(const QString &dbpath)
{
    qCDebug(ServiceLogger) << "Getting database mutex reference for path:" << dbpath;
    QMutexLocker locker(&DbpathMutexMapMutex);

    if (!DbpathMutexMap.contains(dbpath)) {
        qCDebug(ServiceLogger) << "Creating new mutex for database path:" << dbpath;
        DbpathMutexMap.insert(dbpath, SqliteMutex());
    }
    return DbpathMutexMap[dbpath];
}


//日程表
const QString DDataBase::sql_create_schedules =
    " CREATE TABLE if not exists schedules ("
    " scheduleID TEXT not null primary key, "
    " scheduleTypeID TEXT not null,         "
    " summary TEXT not null,                "
    " description TEXT,                     "
    " allDay BOOL not null,                 "
    " dtStart DATETIME not null,            "
    " dtEnd DATETIME not null,              "
    " isAlarm   INTEGER  ,                  "
    " titlePinyin TEXT,                     "
    " isLunar INTEGER not null,             "
    " ics TEXT not null,                    "
    " fileName  TEXT,                       "
    " dtCreate DATETIME not null,           "
    " dtUpdate DATETIME ,                   "
    " dtDelete DATETIME,                    "
    " isDeleted INTEGER not null)";

//类型表
const QString DDataBase::sql_create_scheduleType =
    " CREATE TABLE if not exists scheduleType (            "
    " typeID TEXT not null PRIMARY KEY,                   "
    " typeName TEXT not null,                 "
    " typeDisplayName TEXT,                   "
    " typePath TEXT,                          "
    " typeColorID TEXT not null,           "
    " description TEXT ,                      "
    " privilege INTEGER not null,             "
    " showState INTEGER not null,             "
    " syncTag INTEGER,                        "
    " dtCreate DATETIME not null,             "
    " dtUpdate DATETIME,                      "
    " dtDelete DATETIME,                      "
    " isDeleted INTEGER not null)";

//颜色表
const QString DDataBase::sql_create_typeColor =
    " CREATE TABLE if not exists typeColor (              "
    " ColorID TEXT not null PRIMARY KEY,              "
    " ColorHex TEXT not null,                "
    " privilege INTEGER not null,"
    " dtCreate DATETIME not null)";

// 系统内置日程类型（工作/生活/其他/节假日）用的颜色 ID，
// 取值来自 units.h 的 GTypeColor 表，二者必须对应。
const QString DDataBase::GWorkColorID = "0cecca8a-291b-46e2-bb92-63a527b77d46";
const QString DDataBase::GLifeColorID = "6cfd1459-1085-47e9-8ca6-379d47ec319a";
const QString DDataBase::GOtherColorID = "35e70047-98bb-49b9-8ad8-02d1c942f5d0";
const QString DDataBase::GFestivalColorID = "10af78a1-3c25-4744-91db-6fbe5e88083b";

DDataBase::DDataBase(QObject *parent)
    : QObject(parent)
    , m_DBPath("")
    , m_connectionName("")
{
    qCDebug(ServiceLogger) << "Creating DDataBase instance";
}

DDataBase::~DDataBase()
{
    qCDebug(ServiceLogger) << "Destroying DDataBase instance";
}

QString DDataBase::getDBPath() const
{
    // qCDebug(ServiceLogger) << "Getting database path:" << m_DBPath;
    return m_DBPath;
}

void DDataBase::setDBPath(const QString &DBPath)
{
    // qCDebug(ServiceLogger) << "Setting database path to:" << DBPath;
    m_DBPath = DBPath;
}

QString DDataBase::createUuid()
{
    qCDebug(ServiceLogger) << "Creating new UUID";
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString DDataBase::getConnectionName() const
{
    // qCDebug(ServiceLogger) << "Getting connection name:" << m_connectionName;
    return m_connectionName;
}

void DDataBase::setConnectionName(const QString &connectionName)
{
    // qCDebug(ServiceLogger) << "Setting connection name to:" << connectionName;
    m_connectionName = connectionName;
}

void DDataBase::initDBData()
{
    qCDebug(ServiceLogger) << "Initializing database data";
    createDB();
}

void DDataBase::dbOpen()
{
    qCDebug(ServiceLogger) << "Opening database connection";
    QStringList cntNames = QSqlDatabase::connectionNames();
    if (cntNames.contains(getConnectionName())) {
        qCDebug(ServiceLogger) << "Using existing database connection:" << getConnectionName();
        m_database = QSqlDatabase::database(getConnectionName());
        //如果数据库不一致则设置新的数据库
        if (m_database.databaseName() != getDBPath()) {
            qCDebug(ServiceLogger) << "Database path mismatch, updating to:" << getDBPath();
            m_database.setDatabaseName(getDBPath());
        }
    } else {
        qCDebug(ServiceLogger) << "Creating new database connection:" << getConnectionName();
        m_database = QSqlDatabase::addDatabase("QSQLITE", getConnectionName());
        m_database.setDatabaseName(getDBPath());
        m_database.open();
    }
}

bool DDataBase::dbFileExists()
{
    qCDebug(ServiceLogger) << "Checking if database file exists:" << getDBPath();
    QFile file;
    file.setFileName(getDBPath());
    bool exists = file.exists();
    qCDebug(ServiceLogger) << "Database file exists:" << exists;
    return exists;
}

bool DDataBase::removeDB()
{
    const QString databasePath = getDBPath();
    qCDebug(ServiceLogger) << "Removing database file:" << databasePath;
    if (databasePath.isEmpty() || !QFile::exists(databasePath)) {
        return true;
    }
    if (QFile::remove(databasePath)) {
        return true;
    }
    qCWarning(ServiceLogger) << "Failed to remove database file:" << databasePath;
    return false;
}

void SqliteMutex::lock()
{
    // qCDebug(ServiceLogger) << "Attempting to lock SQLite mutex";
    if (transactionLocked && transactionThreadId == qint64(QThread::currentThreadId())) {
        // qCDebug(ServiceLogger) << "Transaction already locked by current thread, skipping lock";
        return;
    }
    // qCDebug(ServiceLogger) << "Acquiring SQLite mutex lock";
    m.lock();
}

void SqliteMutex::unlock()
{
    // qCDebug(ServiceLogger) << "Attempting to unlock SQLite mutex";
    if (transactionLocked && transactionThreadId == qint64(QThread::currentThreadId())) {
        // qCDebug(ServiceLogger) << "Transaction locked by current thread, skipping unlock";
        return;
    }
    // qCDebug(ServiceLogger) << "Releasing SQLite mutex lock";
    m.unlock();
}

void SqliteMutex::transactionLock()
{
    qCDebug(ServiceLogger) << "Acquiring transaction lock";
    m.lock();
    transactionLocked = true;
    transactionThreadId = qint64(QThread::currentThreadId());
    qCDebug(ServiceLogger) << "Transaction lock acquired for thread:" << transactionThreadId;
}

void SqliteMutex::transactionUnlock()
{
    qCDebug(ServiceLogger) << "Releasing transaction lock for thread:" << transactionThreadId;
    transactionLocked = false;
    transactionThreadId = 0;
    m.unlock();
    qCDebug(ServiceLogger) << "Transaction lock released";
}

SqliteQuery::SqliteQuery(QSqlDatabase db)
    : QSqlQuery(db)
    , _db(db)
{
    qCDebug(ServiceLogger) << "Creating SqliteQuery with database connection";
}

SqliteQuery::SqliteQuery(const QString &connectionName)
    : SqliteQuery(QSqlDatabase::database(connectionName))
{
    qCDebug(ServiceLogger) << "Creating SqliteQuery with connection name:" << connectionName;
}

SqliteQuery::SqliteQuery(const QString &query, QSqlDatabase db)
    : QSqlQuery(query, db)
    , _db(db)
{
    qCDebug(ServiceLogger) << "Creating SqliteQuery with query:" << query;
}

bool SqliteQuery::exec(QString sql)
{
    qCDebug(ServiceLogger) << "Executing SQL query:" << sql;
    getDbMutexRef(_db.databaseName()).lock();
    bool f = QSqlQuery::exec(sql);
    qCDebug(ServiceLogger) << "SQL query execution result:" << f;
    getDbMutexRef(_db.databaseName()).unlock();
    return f;
}

bool SqliteQuery::exec()
{
    qCDebug(ServiceLogger) << "Executing prepared SQL query";
    getDbMutexRef(_db.databaseName()).lock();
    bool f = QSqlQuery::exec();
    qCDebug(ServiceLogger) << "Prepared SQL query execution result:" << f;
    getDbMutexRef(_db.databaseName()).unlock();
    return f;
}

bool SqliteQuery::transaction()
{
    qCDebug(ServiceLogger) << "Starting database transaction";
    getDbMutexRef(_db.databaseName()).transactionLock();
    if (!_db.transaction()) {
        getDbMutexRef(_db.databaseName()).transactionUnlock();
        return false;
    }
    return true;
}

bool SqliteQuery::commit()
{
    qCDebug(ServiceLogger) << "Committing database transaction";
    const bool success = _db.commit();
    if (!success) {
        _db.rollback();
    }
    getDbMutexRef(_db.databaseName()).transactionUnlock();
    return success;
}

void SqliteQuery::rollback()
{
    qCDebug(ServiceLogger) << "Rolling back database transaction";
    _db.rollback();
    getDbMutexRef(_db.databaseName()).transactionUnlock();
}


void SqliteMutex::UnCopyMutex::lock()
{
    // qCDebug(ServiceLogger) << "Acquiring UnCopyMutex lock";
    m.lock();
}

void SqliteMutex::UnCopyMutex::unlock()
{
    // qCDebug(ServiceLogger) << "Releasing UnCopyMutex lock";
    m.unlock();
}
