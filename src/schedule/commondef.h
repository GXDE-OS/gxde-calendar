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
 * Trimmed to the pieces the in-process schedule layer uses.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/commondef.h）。
 */
#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QString>
#include <QStandardPaths>
#include <QLoggingCategory>

// Qt5/Qt6 兼容性宏
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
    #define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

// 参考实现里这两个常量是给 client 进程连 service 进程的 DBus 用的。
// 本项目是单进程，没有 DBus 端点，所以不保留。
// 日志分类沿用参考实现的结构，只是把域名换成本项目自己的：
// 参考的 org.deepin.dde.calendar 挂在 dde-calendar 上，这里跟 gxde-calendar 走。
const QLoggingCategory CommonLogger("org.gxde.calendar");
const QLoggingCategory ClientLogger("org.gxde.calendar.client");
const QLoggingCategory ServiceLogger("org.gxde.calendar.service");
const QLoggingCategory PluginLogger("org.gxde.calendar.plugin");


#endif // COMMONDEF_H
