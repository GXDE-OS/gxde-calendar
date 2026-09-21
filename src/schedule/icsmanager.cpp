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

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
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

// 有 HTTP 状态码时的错误文字。不能用 QNetworkReply::errorString()：那句话是
// 「Error transferring <url> - server replied: <reason phrase>」拼的，而 HTTP/2
// 的响应行里根本没有 reason phrase，拼出来就是「server replied: 」后面空着
// （实测 Google 对私有 ICS 地址限流返回 429 时就是这样，日志里什么也看不出来）。
// 429/403/404 这几个订阅最常见的给一句人话，其余至少把状态码带上。
QString httpErrorText(int status)
{
    switch (status) {
    case 403:
        return QCoreApplication::translate("IcsManager", "Access denied (HTTP 403)");
    case 404:
        return QCoreApplication::translate("IcsManager",
                                           "Not found (HTTP 404), check the subscription address");
    case 429:
        return QCoreApplication::translate(
                "IcsManager",
                "Too many requests (HTTP 429), the server is rate limiting, try again later");
    default:
        return QCoreApplication::translate("IcsManager", "Server returned HTTP %1").arg(status);
    }
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

IcsManager::IcsFileInfo IcsManager::readFileInfo(const QString &icsFilePath) const
{
    IcsFileInfo info;

    if (icsFilePath.isEmpty() || !QFile::exists(icsFilePath)) {
        return info;
    }

    KCalendarCore::ICalFormat icalformat;
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(currentTimeZone()));
    if (!icalformat.load(cal, icsFilePath)) {
        qCWarning(ServiceLogger) << "Failed to read ICS file:" << icsFilePath;
        return info;
    }

    info.valid = true;
    info.typeName = cal->nonKDECustomProperty("X-DDE-CALENDAR-TYPE-NAME");
    if (info.typeName.isEmpty()) {
        info.typeName = cal->nonKDECustomProperty("X-WR-CALNAME");
    }
    info.colorCode = cal->nonKDECustomProperty("X-DDE-CALENDAR-TYPE-COLOR");
    info.eventCount = cal->events().count();
    return info;
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
        //拿到状态码就自己拼错误文字（原因见 httpErrorText()）：没有状态码说明压根
        //没收到响应（DNS 解析不了、超时、TLS 失败），这时 errorString() 是有内容的
        const QString error = (status > 0) ? httpErrorText(status) : reply->errorString();
        qCWarning(ServiceLogger) << "Failed to fetch ICS from" << reply->url()
                                 << "status" << status << ":" << error;
        emit fetchFinished(token, false, false, QByteArray(), QString(), error);
        return;
    }

    const QByteArray data = reply->readAll();

    //拉回来的东西不是 ICS 就别往下走。最典型的坑是把浏览器地址栏里的日历网页地址
    //（比如 calendar.google.com/calendar/u/0?cid=...）当成订阅地址粘进来：服务端 200
    // 返回一个登录页，KCalendarCore 解析失败时会把整页 HTML 打进日志（一页几百 KB），
    //用户看到的也只有一句 parse error。这里先看内容，给一条能看懂的错误。
    if (!data.contains("BEGIN:VCALENDAR")) {
        const bool looksLikeHtml = reply->header(QNetworkRequest::ContentTypeHeader)
                                           .toString()
                                           .contains(QStringLiteral("html"), Qt::CaseInsensitive)
                || data.left(512).trimmed().startsWith('<');
        QString error = looksLikeHtml
                ? tr("This address returned a web page, not an ICS feed")
                : tr("The content returned is not in ICS format");
        qCWarning(ServiceLogger) << "ICS subscription content rejected:" << reply->url()
                                 << (looksLikeHtml ? "html page" : "not ics") << data.size() << "bytes";
        emit fetchFinished(token, false, false, QByteArray(), QString(), error);
        return;
    }

    const QString etag = QString::fromUtf8(reply->rawHeader("ETag"));
    qCDebug(ServiceLogger) << "Fetched" << data.size() << "bytes from" << reply->url();
    emit fetchFinished(token, true, false, data, etag, QString());
}
