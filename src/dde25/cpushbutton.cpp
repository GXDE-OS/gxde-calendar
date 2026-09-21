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
 */

#include "cpushbutton.h"

#include "dde25common.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

CPushButton::CPushButton(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layoutAddType = new QHBoxLayout();
    m_textLabel = new QLabel(tr("New event type"));

    m_textLabel->setFixedSize(100, 34);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_textLabel->setFixedSize(200, 34);
    layoutAddType->setSpacing(0);
    layoutAddType->setContentsMargins(0, 0, 0, 0);
    layoutAddType->setAlignment(Qt::AlignLeft);
    m_iconButton = new QToolButton(this);
    m_iconButton->setObjectName("IconButton");
    m_iconButton->setAccessibleName("IconButton");
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setFixedSize(16, 16);
    m_iconButton->setIconSize(QSize(14, 14));
    m_iconButton->setAutoRaise(true);   //QToolButton 没有 setFlat，用 autoRaise 达到同样的平坦效果

    QPalette pa = m_textLabel->palette();

    //设置深浅色主题下正常状态时的文本颜色，与下拉框颜色对其
    if (DDE25::themeType() == 2) {
        pa.setBrush(QPalette::WindowText, QColor("#FFFFFF"));
    } else {
        pa.setBrush(QPalette::WindowText, QColor("#000000"));
    }
    m_textLabel->setPalette(pa);

    layoutAddType->setContentsMargins(33, 0, 0, 0);
    layoutAddType->addWidget(m_iconButton);
    layoutAddType->addSpacing(5);
    layoutAddType->addWidget(m_textLabel);
    setFixedHeight(34);
    setLayout(layoutAddType);
}

void CPushButton::setHighlight(bool status)
{
    if (status == m_Highlighted) {
        return;
    }
    m_Highlighted = status;
    update();
}

bool CPushButton::isHighlight()
{
    return m_Highlighted;
}

void CPushButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_pressed = true;
}

void CPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed && rect().contains(event->pos())) {
        emit clicked();
    }
    m_pressed = false;
}

void CPushButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (m_Highlighted) {
        //背景设置为高亮色，此时图标要用白色那份
        m_iconButton->setIcon(QIcon(":/resources/icon/dde_calendar_create_dark.svg"));
        painter.setBrush(palette().highlight());
        m_textLabel->setBackgroundRole(QPalette::Highlight);
    } else {
        //背景透明，图标跟随深浅主题
        m_iconButton->setIcon(QIcon(DDE25::themeType() == 2
                                        ? ":/resources/icon/dde_calendar_create_dark.svg"
                                        : ":/resources/icon/dde_calendar_create_light.svg"));
        painter.setBrush(QBrush("#00000000"));
        m_textLabel->setBackgroundRole(QPalette::Window);
    }
    //绘制背景
    painter.drawRect(this->rect());
}
