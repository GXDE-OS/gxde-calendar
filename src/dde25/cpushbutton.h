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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cpushbutton.*）。
 *
 * 差异：
 * 1. DTK2Widget 没有 DIconButton，图标按钮换成 QToolButton；
 * 2. DPaletteHelper 不在 include 路径里，取色直接走 widget 自己的 palette；
 * 3. DGuiApplicationHelper::themeType() 换成 DDE25::themeType()；
 * 4. 参考实现正常态的图标来自 DTK 内置图标主题（"dde_calendar_create"），本机没装，
 *    改成 qrc 里内置的深浅色两份 svg（取自 dde-calendar assets）。
 */

#ifndef CPUSHBUTTON_H
#define CPUSHBUTTON_H

#include <QLabel>
#include <QToolButton>
#include <QWidget>

class CPushButton : public QWidget
{
    Q_OBJECT
public:
    explicit CPushButton(QWidget *parent = nullptr);

    //设置高亮状态
    void setHighlight(bool status);
    //返回高亮状态
    bool isHighlight();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_Highlighted = false; //记录当前是否处于高亮状态
    bool m_pressed = false; //记录鼠标是否按下
    QLabel *m_textLabel = nullptr;  //文字控件
    QToolButton *m_iconButton = nullptr; //icon控件
};

#endif // CPUSHBUTTON_H
