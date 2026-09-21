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
 *
 * 日程块的坐标换算：把「时间区间」映射成视图里的矩形。月视图按天分列，
 * 周/日视图按一天 24 小时纵向铺开，两者共用这一套。
 */

#ifndef SCHEDULECOORMANAGE_H
#define SCHEDULECOORMANAGE_H

#include <QDate>
#include <QDateTime>
#include <QRect>

class CScheduleCoorManage
{
public:
    CScheduleCoorManage() = default;
    ~CScheduleCoorManage() = default;

    // 设定绘制区域：宽高 + 日期范围 + 右侧留白
    void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin);
    void setDateRange(QDate begindate, QDate enddate);

    int getRightMagin() const { return m_rightmagin; }

    QRectF getDrawRegion(QDateTime begintime, QDateTime endtime);
    QRectF getDrawRegion(QDateTime begintime, QDateTime endtime, int index, int coount);
    QRectF getDrawRegion(QDate date, QDateTime begintime, QDateTime endtime,
                         int index, int coount, int maxNum, int type = 0);
    QRectF getDrawRegionF(QDateTime begintime, QDateTime endtime);
    QRectF getAllDayDrawRegion(QDate begin, QDate end);

    // 由坐标反查时间/日期（右键新建日程要用）
    QDateTime getDate(QPointF pos);
    QDate getsDate(QPointF pos);

    float getHeight(const QTime &time) const;

    QDate getBegindate() const { return m_begindate; }

private:
    int m_width {0};
    int m_height {0};
    QDate m_begindate;
    QDate m_enddate;
    qint64 m_totalDay {0};
    int m_rightmagin = 0;
};

#endif // SCHEDULECOORMANAGE_H
