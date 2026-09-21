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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cradiobutton.*）。
 *
 * 差异：参考实现用 DPaletteHelper 取高亮色画选中态，本项目没把 dtk6/DWidget 加进
 * include 路径，改用 dde25common.h 里现成的 DDE25::systemActiveColor()——同项目
 * 其它地方取系统活动色也走它。
 */

#ifndef CRADIOBUTTON_H
#define CRADIOBUTTON_H

#include <QRadioButton>

//自定义颜色单选按钮
class CRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit CRadioButton(QWidget *parent = nullptr);

    void setColor(const QColor &color);
    QColor getColor();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
};

#endif // CRADIOBUTTON_H
