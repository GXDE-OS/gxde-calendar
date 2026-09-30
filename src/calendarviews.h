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
 */

#ifndef SRC_CALENDARVIEWS_H_
#define SRC_CALENDARVIEWS_H_

#include <QWidget>
#include <QDate>

class QLabel;
class QPushButton;
class QFrame;

class YearView : public QWidget {
    Q_OBJECT
public:
    explicit YearView(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setFirstWeekday(int weekday);

signals:
    void dateClicked(const QDate &date);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QRect monthRect(int monthIndex) const;
    int firstWeekdayOffset(const QDate &firstDay) const;
    void switchYear(int offset);
    void updateYearLabel();

    QDate m_currentDate;
    int m_firstWeekDay = 0;

    QLabel *m_yearLabel = nullptr;
    QPushButton *m_prevButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_todayButton = nullptr;
    QFrame *m_todayFrame = nullptr;
};


class WeekView : public QWidget {
    Q_OBJECT
public:
    explicit WeekView(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setFirstWeekday(int weekday);

signals:
    void dateClicked(const QDate &date);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QDate weekStart() const;

    QDate m_currentDate;
    int m_firstWeekDay = 0;
};

class DayView : public QWidget {
    Q_OBJECT
public:
    explicit DayView(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QDate m_currentDate;
};

#endif  // SRC_CALENDARVIEWS_H_
