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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/lunarcalendarwidget.*）。
 *
 * 差异：
 * 1. 黄历数据从 LunarManager 的 DBus 调用改成 DDE25::LunarCache（同样是按月缓存，
 *    提前 ensureMonth 一次，绘制时只读缓存）；
 * 2. DPaletteHelper 不在本项目的 include 路径里，CalenderStyle 的取色直接走
 *    QPalette；DPalette::FrameBorder 对应到 QPalette::Mid；
 * 3. DStyle::pixelMetric(PM_ContentsMargins) 换成 QStyle::PM_LayoutLeftMargin
 *    （DTK2Widget 没有 DStyle 头）；
 * 4. 底部留白 QLabel 的 objectName 原来误写在 m_lunarLabel 上，这里改成写在
 *    留白 QLabel 自己身上（两个名字都没有被样式表引用，不影响外观）。
 */

#ifndef LUNARCALENDARWIDGET_H
#define LUNARCALENDARWIDGET_H

#include <QCalendarWidget>

class QLabel;
class QStyle;

class LunarCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit LunarCalendarWidget(QWidget *parent = nullptr);
    ~LunarCalendarWidget() override;

    /**
     * @brief setLunarYearText 设置阴历年描述语
     * @param text
     */
    void setLunarYearText(const QString &text);
    /**
     * @brief lunarYearText 返回阴历年描述语
     * @return
     */
    QString lunarYearText();

protected:
    QSize minimumSizeHint() const override;

private:
    QLabel *m_lunarLabel   = nullptr;//阴历年的label
    QStyle *m_style        = nullptr;//日历视图的样式对象（不归 QWidget 所有，析构里手动删）
};

#endif // LUNARCALENDARWIDGET_H
