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
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/dataManage/schedulecoormanage.*）。
 */

#include "schedulecoormanage.h"

#include <QTime>

void CScheduleCoorManage::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    m_width = w;
    m_height = h;
    m_rightmagin = rightmagin;
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
}

void CScheduleCoorManage::setDateRange(QDate begindate, QDate enddate)
{
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime)
{
    QRectF rect;
    if (begintime > endtime) {
        return rect;
    }

    const QDate begindate = begintime.date();
    const QDate enddate = endtime.date();
    const QTime beginzero(0, 0, 0);
    const QTime beginScheduleT = begintime.time();
    const QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        return rect;
    }

    const qint64 beginday = m_begindate.daysTo(begindate) + 1;
    const qint64 day = begindate.daysTo(enddate) + 1;
    const int scheduleBT = beginzero.secsTo(beginScheduleT);
    const int scheduleET = beginzero.secsTo(endScheduleT);
    const qreal rWidth = m_width * (1.0 * day / m_totalDay);
    const qreal rHeight = m_height * ((scheduleET - scheduleBT) / 86400.0);
    const qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    const qreal posY = m_height * (scheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime, int index, int coount)
{
    QRectF rect;
    if (begintime > endtime) {
        return rect;
    }

    const QDate begindate = begintime.date();
    const QDate enddate = endtime.date();
    const QTime beginzero(0, 0, 0);
    const QTime beginScheduleT = begintime.time();
    const QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        return rect;
    }

    const qint64 beginday = m_begindate.daysTo(begindate) + 1;
    const qint64 day = begindate.daysTo(enddate) + 1;
    const int scheduleBT = beginzero.secsTo(beginScheduleT);
    const int scheduleET = beginzero.secsTo(endScheduleT);
    const qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    const qreal rHeight = m_height * ((scheduleET - scheduleBT) / 86400.0);
    const qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    const qreal posY = m_height * (scheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDate date, QDateTime begintime, QDateTime endtime,
                                          int index, int coount, int maxNum, int type)
{
    QRectF rect;
    if (begintime > endtime) {
        return rect;
    }

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    const QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    // 跨天日程被裁到目标这一天：起点早于当天就压到 00:00，终点晚于当天就压到 23:59:59
    if (begindate < date) {
        begindate = date;
        beginScheduleT = beginzero;
    }
    if (enddate > date) {
        enddate = date;
        endScheduleT = QTime(23, 59, 59);
    }

    const qint64 beginday = m_begindate.daysTo(begindate) + 1;
    const qint64 day = begindate.daysTo(enddate) + 1;
    const int scheduleBT = beginzero.secsTo(beginScheduleT);
    const int scheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    qreal rHeight = m_height * ((scheduleET - scheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    qreal posY = m_height * (scheduleBT / 86400.0);

    // 重叠日程多到超过 maxNum 时不再等分，改成固定 27px 一列并排，多出来的叠在最后
    if (coount > maxNum && type == 0) {
        const qreal sscale = 27.0 / (m_width * (1.0 * day / m_totalDay));

        if (index < maxNum + 1) {
            rWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
        } else {
            const qreal trWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            rWidth = m_width * (1.0 * day / m_totalDay) - (index - 1) * trWidth;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * trWidth;
        }
    }

    // 太矮的块撑到 20px，免得文字压成一团；贴底时整体上移
    if (rHeight < 20) {
        if (posY + 20 > m_height) {
            posY = m_height - 20;
        }
        rHeight = 20;
    }

    if (posX < 1) {
        posX = 1;
        rWidth = rWidth - posX;
    }

    rect = QRectF(posX, posY, rWidth, rHeight);
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegionF(QDateTime begintime, QDateTime endtime)
{
    QRectF rectf;
    if (begintime > endtime) {
        return rectf;
    }

    const QDate begindate = begintime.date();
    const QDate enddate = endtime.date();
    const QTime beginzero(0, 0, 0);
    const QTime beginScheduleT = begintime.time();
    const QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        return rectf;
    }

    const qint64 beginday = m_begindate.daysTo(begindate) + 1;
    const qint64 day = begindate.daysTo(enddate) + 1;
    const int scheduleBT = beginzero.secsTo(beginScheduleT);
    const int scheduleET = beginzero.secsTo(endScheduleT);
    const qreal rWidth = m_width * (1.0 * day / m_totalDay);
    const qreal rHeight = m_height * ((scheduleET - scheduleBT) / 86400.0);
    const qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    const qreal posY = m_height * (scheduleBT / 86400.0);
    rectf = QRectF(posX, posY, rWidth, rHeight);
    return rectf;
}

QRectF CScheduleCoorManage::getAllDayDrawRegion(QDate begin, QDate end)
{
    QRectF rect;
    if (begin > end) {
        return rect;
    }

    QDate begindate = begin;
    QDate enddate = end;

    if (begindate < m_begindate) {
        begindate = m_begindate;
    }
    if (enddate > m_enddate) {
        enddate = m_enddate;
    }

    const qint64 beginday = m_begindate.daysTo(begindate);
    const qint64 day = begindate.daysTo(enddate) + 1;
    const qreal rWidth = m_width * (1.0 * day / m_totalDay) - 12;
    const qreal rHeight = m_height;
    const qreal posX = m_width * (1.0 * beginday / m_totalDay);
    const qreal posY = 0;
    rect = QRectF(posX + 6, posY, rWidth - m_rightmagin, rHeight);
    return rect;
}

QDateTime CScheduleCoorManage::getDate(QPointF pos)
{
    QDateTime begintime;
    qint64 day = static_cast<qint64>((1.0 * pos.x() / m_width) * m_totalDay);

    if (day < 0) {
        day = 0;
    } else if (day >= m_totalDay) {
        day = m_totalDay - 1;
    }
    const int time = static_cast<int>((1.0 * pos.y() / m_height) * 86400.0);
    const int hours = time / 3600;
    const int minutes = (time - 3600 * hours) / 60;
    const int secss = time - 3600 * hours - 60 * minutes;
    const QDate date = m_begindate.addDays(day);
    begintime.setDate(date);
    begintime.setTime(QTime(hours, minutes, secss));
    return begintime;
}

QDate CScheduleCoorManage::getsDate(QPointF pos)
{
    qint64 day = static_cast<qint64>((1.0 * pos.x() / m_width) * m_totalDay);

    if (day < 0) {
        day = 0;
    } else if (day >= m_totalDay) {
        day = m_totalDay - 1;
    }
    return m_begindate.addDays(day);
}

float CScheduleCoorManage::getHeight(const QTime &time) const
{
    const QTime beginzero(0, 0, 0);
    const int scheduleBT = beginzero.secsTo(time);
    return static_cast<float>(m_height * (scheduleBT / 86400.0));
}
