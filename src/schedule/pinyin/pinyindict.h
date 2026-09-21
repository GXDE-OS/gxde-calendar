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
 * Verbatim.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/pinyin/pinyindict.h）。
 */
#ifndef PINYINDICT_H
#define PINYINDICT_H

#include <QMap>
#include <QVector>

//获取拼音字典
const QVector<QMap<int,QString> > getPinYinDictVector();
// 带音标字符。
extern QMap<QString, QString> phoneticSymbol;
/* 合法拼音列表 */
extern QVector<QString> validPinyinList;

#endif // PINYINDICT_H
