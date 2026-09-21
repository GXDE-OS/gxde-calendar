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

#include "icsmanager.h"

#include "commondef.h"

#include "icalformat.h"
#include "memorycalendar.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimeZone>
#include <QUrl>

namespace {

// 参考实现用当前时区建 MemoryCalendar，解析无 TZID 的浮动时间时要有个基准。
QTimeZone currentTimeZone()
{
    return QDateTime::currentDateTime().timeZone();
}

} // namespace

IcsManager::IcsManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
}

IcsManager::~IcsManager() = default;

DSchedule::List IcsManager::loadFromFile(const QString &icsFilePath, bool *ok) const
{
    if (ok) {
        *ok = false;
    }

    KCalendarCore::ICalFormat icalformat;
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(currentTimeZone()));
    if (!icalformat.load(cal, icsFilePath)) {
        qCWarning(ServiceLogger) << "Failed to load ICS file:" << icsFilePath;
        return {};
    }

    if (ok) {
        *ok = true;
    }

    DSchedule::List schedules;
    const KCalendarCore::Event::List events = cal->events();
    schedules.reserve(events.size());
    for (const KCalendarCore::Event::Ptr &event : events) {
        schedules.append(DSchedule::Ptr(new DSchedule(*event.data())));
    }

    qCDebug(ServiceLogger) << "Loaded" << schedules.size() << "events from" << icsFilePath;
    return schedules;
}

DSchedule::List IcsManager::loadFromData(const QByteArray &data, bool *ok) const
{
    if (ok) {
        *ok = false;
    }
    if (data.isEmpty()) {
        return {};
    }

    KCalendarCore::ICalFormat icalformat;
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(currentTimeZone()));
    // 用 fromRawString 而不是 fromString：订阅拉回来的是字节流，
    // 交给 KCalendarCore 按 ICS 自己的编码规则处理，不先假定是 UTF-8。
    if (!icalformat.fromRawString(cal, data)) {
        qCWarning(ServiceLogger) << "Failed to parse ICS payload, size:" << data.size();
        return {};
    }

    if (ok) {
        *ok = true;
    }

    DSchedule::List schedules;
    const KCalendarCore::Event::List events = cal->events();
    schedules.reserve(events.size());
    for (const KCalendarCore::Event::Ptr &event : events) {
        schedules.append(DSchedule::Ptr(new DSchedule(*event.data())));
    }
    return schedules;
}

bool IcsManager::saveToFile(const QString &icsFilePath, const DSchedule::List &schedules,
                            const QString &typeID, const QString &typeName,
                            const QString &typeColor) const
{
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(nullptr));

    // 参考实现在导出时把日程类型的身份挂成扩展属性，导回来能还原类型。
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-ID", typeID);
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-NAME", typeName);
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-COLOR", typeColor);
    cal->setNonKDECustomProperty("X-WR-CALNAME", typeName);

    for (const DSchedule::Ptr &schedule : schedules) {
        if (!schedule.isNull()) {
            cal->addEvent(schedule);
        }
    }

    KCalendarCore::ICalFormat icalformat;
    const bool ok = icalformat.save(cal, icsFilePath);
    if (!ok) {
        qCWarning(ServiceLogger) << "Failed to write ICS file:" << icsFilePath;
    }
    return ok;
}

void IcsManager::fetch(const QString &url, const QString &etag, const QString &token)
{
    const QUrl target(url);
    if (!target.isValid() || target.scheme().isEmpty()) {
        qCWarning(ServiceLogger) << "Invalid ICS subscription url:" << url;
        emit fetchFinished(token, false, false, QByteArray(), QString(), tr("Invalid URL"));
        return;
    }

    QNetworkRequest request(target);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    // 有些日历服务（比如 GitHub 的 raw 地址）会对没有 UA 的请求直接拒绝
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("gxde-calendar"));
    if (!etag.isEmpty()) {
        request.setRawHeader("If-None-Match", etag.toUtf8());
    }

    const bool hadETag = !etag.isEmpty();
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, hadETag, token] {
        onReplyFinished(reply, hadETag, token);
    });
}

void IcsManager::onReplyFinished(QNetworkReply *reply, bool hadETag, const QString &token)
{
    reply->deleteLater();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // 304：内容没变，不用重新导入
    if (hadETag && status == 304) {
        qCDebug(ServiceLogger) << "ICS subscription not modified, skipping" << reply->url();
        emit fetchFinished(token, true, true, QByteArray(), QString(), QString());
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        qCWarning(ServiceLogger) << "Failed to fetch ICS from" << reply->url() << ":" << error;
        emit fetchFinished(token, false, false, QByteArray(), QString(), error);
        return;
    }

    const QByteArray data = reply->readAll();
    const QString etag = QString::fromUtf8(reply->rawHeader("ETag"));
    qCDebug(ServiceLogger) << "Fetched" << data.size() << "bytes from" << reply->url();
    emit fetchFinished(token, true, false, data, etag, QString());
}
