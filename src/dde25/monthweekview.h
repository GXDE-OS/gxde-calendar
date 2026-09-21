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
 * Origin copyright bearer: 2017 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthweekview.*）。
 */

#ifndef MONTHWEEKVIEW_H
#define MONTHWEEKVIEW_H

#include <QColor>
#include <QDate>
#include <QFont>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>

class WeekRect;

/**
 * @brief 月视图的星期标题行
 */
class CMonthWeekView : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthWeekView(QWidget *parent = nullptr);
    ~CMonthWeekView() override;

    void setFirstDay(const Qt::DayOfWeek weekday);
    void setTheMe(int type = 0);
    void updateWeek();
    // 设置当前时间
    void setCurrentDate(const QDate &currentDate);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_backgroundColor;
    QVector<WeekRect *> m_weekRect;
    int m_radius = 8;
    Qt::DayOfWeek m_firstWeek{Qt::Sunday};
    Qt::DayOfWeek m_currentWeek{Qt::Monday};
};

class WeekRect
{
public:
    WeekRect();
    void setWeek(const Qt::DayOfWeek &showWeek, const bool &showLine = false);
    void setRect(const QRectF &rectF);
    void paintRect(QPainter &painter);
    void setTheMe(int type = 0);

private:
    QColor m_activeColor;
    QColor m_testColor;
    Qt::DayOfWeek m_showWeek;
    QString m_weekStr;
    QFont m_font;
    QRectF m_rectF;
    bool m_showLine;
    qreal m_lineHeight{2};
};

#endif // MONTHWEEKVIEW_H
