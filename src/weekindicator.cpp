/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#include "weekindicator.h"

#include <QLabel>
#include <QDebug>
#include <QDate>

WeekIndicator::WeekIndicator(QWidget *parent) : QWidget(parent)
{
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    setLayout(m_mainLayout);
}

void WeekIndicator::setList(int weekday)
{
    QLayoutItem* child;
    while((child = m_mainLayout->takeAt(0)) != 0) {
        if(child->widget() != 0) {
            delete child->widget();
        }
        delete child;
    }

    QLocale locale;
    for (int i = 0; i != 7; ++i) {

        int d = checkDay(i - weekday);

        QLabel *label = new QLabel(locale.dayName(d ? d : 7, QLocale::ShortFormat));

        if ((i == weekday - 1 && weekday != 0) || i == weekday || (weekday == 0 && i == 6)) {
            label->setObjectName("CalendarHeaderWeekend");
        } else {
            label->setObjectName("CalendarHeaderWeekday");
        }

        label->setAlignment(Qt::AlignCenter);
        label->setFixedSize(m_cellWidth, DDECalendar::HeaderItemHeight);
        m_mainLayout->addWidget(label, 0, Qt::AlignCenter);
    }
}

void WeekIndicator::setCellWidth(int width) {
    if (m_cellWidth == width) {
        return;
    }

    m_cellWidth = width;
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        if (QWidget *label = m_mainLayout->itemAt(i)->widget()) {
            label->setFixedSize(m_cellWidth, DDECalendar::HeaderItemHeight);
        }
    }
}

int WeekIndicator::checkDay(int weekday) {

    // check the week, calculate the correct order in the custom.

    if (weekday <= 0)
        return weekday += 7;

    if (weekday > 7)
        return weekday -= 7;

    return weekday;
}
