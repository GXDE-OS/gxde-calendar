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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cweekwidget.*）。
 * 差异：去掉 CalendarManager 依赖，一周起始日由 setFirstDay() 直接指定；
 *       字体也不再随语言环境做额外缩放调整以外的事情（保持原样的英文缩放）。
 */

#ifndef CWEEKWIDGET_H
#define CWEEKWIDGET_H

#include <QPushButton>

/**
 * @brief The CWeekWidget class
 * 日视图侧栏里的星期名行（按当前一周起始日排布「一 二 三 …」）
 */
class CWeekWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit CWeekWidget(QWidget *parent = nullptr);

    // 设置一周首日
    void setFirstDay(Qt::DayOfWeek);
    // 设置是否根据配置自动设置（gxde 无 CalendarManager，此开关仅为兼容保留）
    void setAutoFirstDay(bool);
    // 设置字体大小是否跟随界面大小
    void setAutoFontSizeByWindow(bool);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_autoFirstDay = true;        // 是否根据配置自动设置
    bool m_autoFontSizeByWindow = true; // 字体大小是否跟随界面大小

    Qt::DayOfWeek m_firstDay = Qt::Monday; // 一周首日
};

#endif // CWEEKWIDGET_H
