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
 * Trimmed to match units.h.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/units.cpp）。
 */
#include "units.h"
#include "commondef.h"
#include <QDir>
#include <QFile>

#include <QTimeZone>
#include <QStandardPaths>
#include <QDebug>

QString dtToString(const QDateTime &dt)
{
    // qCDebug(CommonLogger) << "Converting QDateTime to string:" << dt;
    //空时间直接给空串。下面的 dt.toString() 对空时间是空串，只剩时区尾巴拼出来，
    //落库就是 "+06:00" 这种脏数据（dtUpdate/dtDelete/lastSync 都可能是空时间）。
    if (!dt.isValid()) {
        return QString();
    }

    const int offsetSeconds = dt.timeZone().offsetFromUtc(dt);
    const QChar sign = offsetSeconds < 0 ? QLatin1Char('-') : QLatin1Char('+');
    const int absoluteSeconds = qAbs(offsetSeconds);
    const int hours = absoluteSeconds / 3600;
    const int minutes = (absoluteSeconds % 3600) / 60;
    return QStringLiteral("%1%2%3:%4")
        .arg(dt.toString(QStringLiteral("yyyy-MM-ddThh:mm:ss")))
        .arg(sign)
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'));
}

QDateTime dtConvert(const QDateTime &datetime)
{
    // qCDebug(CommonLogger) << "Converting QDateTime:" << datetime;
    QDateTime dt = datetime;
    dt.setOffsetFromUtc(dt.offsetFromUtc());
    return dt;
}

QDateTime dtFromString(const QString &st)
{
    // 保留字符串中的时区/UTC 偏移，显示层再根据当前系统时区转换。
    // 不能在这里转成本地时间，否则用户修改系统时区后会丢失原始时刻。
    return QDateTime::fromString(st, Qt::ISODate);
}

QString getDBPath()
{
    // 参考实现这里是 getHomeConfigPath() + "/deepin/dde-calendar-service"，
    // 是 service 进程的账户库目录。本项目单进程、只有一个库文件，
    // 直接给到文件。
    // 目录与 holidayapi.cpp 里硬编码的 ~/.config/GXDE/gxde-calendar 保持一致。
    const QDir dir(getHomeConfigPath() + QStringLiteral("/GXDE/gxde-calendar"));
    if (!dir.exists()) {
        QDir().mkpath(dir.absolutePath());
    }
    return dir.absoluteFilePath(QStringLiteral("schedule.db"));
}

QDate dateFromString(const QString &date)
{
    // qCDebug(CommonLogger) << "Converting string to QDate:" << date;
    return QDate::fromString(date, Qt::ISODate);
}

QString dateToString(const QDate &date)
{
    // qCDebug(CommonLogger) << "Converting QDate to string:" << date;
    return date.toString("yyyy-MM-dd");
}

QString getHomeConfigPath()
{
    qCDebug(CommonLogger) << "Getting home config path.";
    //根据环境变量获取config目录
    QString configPath = QString(qgetenv("XDG_CONFIG_HOME"));
    if(configPath.trimmed().isEmpty()) {
        qCDebug(CommonLogger) << "XDG_CONFIG_HOME is empty, using QStandardPaths.";
        configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    }
    return configPath;
}

bool withinTimeFrame(const QDate &date)
{
    // qCDebug(CommonLogger) << "Checking if date" << date << "is within time frame.";
    return date.isValid() && (date.year() >= 1900 && date.year() <=2100);
}
