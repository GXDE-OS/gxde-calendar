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
 * Trimmed: dropped CalDAV palette and the DBus/进程 helpers that the single-process port has no use for.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/units.h）。
 */
#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QDir>

// 系统内置的九个日程类型颜色（参考实现 units.h 的 GTypeColor）。
// 这几个 UUID 是参考实现里系统日程类型的固定 ID，颜色表的 privilege=1 行按它们建。
static const QMap<QString, QString> GTypeColor = {
    {"0cecca8a-291b-46e2-bb92-63a527b77d46", "#FF5E97"},
    {"10af78a1-3c25-4744-91db-6fbe5e88083b", "#FF9436"},
    {"263d6c79-32b6-4b00-bf0d-741e50a9550f", "#FFDC00"},
    {"35e70047-98bb-49b9-8ad8-02d1c942f5d0", "#5BDD80"},
    {"406fc0df-87ce-4b3f-b1bc-65d89d791dbc", "#00B99B"},
    {"5bf13e88-e99f-4975-80a8-149fe0a315e3", "#4293FF"},
    {"6cfd1459-1085-47e9-8ca6-379d47ec319a", "#5D51FF"},
    {"70080e96-e68d-40af-9cca-2f41021f6142", "#A950FF"},
    {"8ac5c8bb-55ce-4264-8b0a-5d32116cf983", "#717171"}};

QString dtToString(const QDateTime &dt);
QDateTime dtFromString(const QString &st);

QString dateToString(const QDate &date);
QDate dateFromString(const QString &date);

//日程库文件路径（区别于参考实现的 service 账户库）
QString getDBPath();

//获取家配置目录
QString getHomeConfigPath();

//时间转换
QDateTime dtConvert(const QDateTime &datetime);

//是否在显示时间范围内1900-2100
bool withinTimeFrame(const QDate &date);


#endif // UNITS_H
