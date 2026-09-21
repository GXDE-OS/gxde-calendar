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
 * The .ics file import/export half is ported from DDE calendar tag 6.6.0
 * (upstream license: LGPL-3.0-or-later, relicensed under GPL-3.0-or-later as
 * permitted by LGPL-3.0 section 3).  The remote-URL subscription half does not
 * exist upstream and is written for this project.
 * ----------------------------------------------------------------------------
 * 文件导入/导出移植自 dde-calendar（src/calendar-service/src/calendarDataManager/
 * daccountmodule.cpp 的 importSchedule / exportSchedule）。
 * 远程 URL 订阅是新增的：参考实现只有手工选文件的导入/导出，没有订阅。
 */

#ifndef ICSMANAGER_H
#define ICSMANAGER_H

#include "dschedule.h"

#include <QByteArray>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * @brief .ics 的读写，以及远程订阅的拉取。
 *
 * 这一层不碰数据库：只负责「ICS 文本 <-> DSchedule 列表」和「把 URL 拉下来」，
 * 入库由 CalendarService 负责。这样解析逻辑能单独测，也免得网络回调里写库。
 */
class IcsManager : public QObject
{
    Q_OBJECT
public:
    explicit IcsManager(QObject *parent = nullptr);
    ~IcsManager() override;

    /**
     * @brief loadFromFile     读取本地 .ics 文件
     * @param icsFilePath      文件路径
     * @param ok               可为空；写入是否成功
     */
    DSchedule::List loadFromFile(const QString &icsFilePath, bool *ok = nullptr) const;

    /**
     * @brief loadFromData     解析一段 ICS 文本（订阅拉回来的内容走这里）
     */
    DSchedule::List loadFromData(const QByteArray &data, bool *ok = nullptr) const;

    /**
     * @brief saveToFile       把日程写成 .ics 文件
     *
     * 参考实现在日历对象上挂 X-DDE-CALENDAR-TYPE-ID / -NAME / -COLOR 和
     * X-WR-CALNAME，这里照做，导出的文件再导回来能还原出日程类型的名字和颜色。
     */
    bool saveToFile(const QString &icsFilePath, const DSchedule::List &schedules,
                    const QString &typeID, const QString &typeName,
                    const QString &typeColor) const;

    /**
     * @brief fetch            发起一次订阅拉取（异步）
     * @param url              远程 .ics 地址
     * @param etag             上次拿到的 ETag，非空时带 If-None-Match；
     *                         服务端返回 304 时 notModified 为 true
     * @param token            调用方自己的标识，原样回传到 fetchFinished。
     *                         一个 manager 上可以同时有多个在飞的请求，
     *                         调用方靠它认出这次响应属于哪个订阅。
     *
     * 结束后总是发 fetchFinished。
     */
    void fetch(const QString &url, const QString &etag = QString(), const QString &token = QString());

signals:
    void fetchFinished(const QString &token, bool ok, bool notModified, const QByteArray &data,
                       const QString &etag, const QString &error);

private:
    void onReplyFinished(QNetworkReply *reply, bool hadETag, const QString &token);

    QNetworkAccessManager *m_network = nullptr;
};

#endif // ICSMANAGER_H
