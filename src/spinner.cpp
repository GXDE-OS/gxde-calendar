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

#include "spinner.h"
#include "dde25/dde25common.h"

#include <QLabel>
#include <QHBoxLayout>

Spinner::Spinner(QWidget *parent) :
    QWidget(parent),
    m_prevButton(new DImageButton),
    m_nextButton(new DImageButton),
    m_label(new QLabel)
{
    setFixedHeight(23);

    setPrevButtonDisabled(false);
    setNextButtonDisabled(false);

    m_label->setFixedWidth(40);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setText(QString::number(m_value));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_prevButton);
    layout->addWidget(m_label);
    layout->addWidget(m_nextButton);
    setLayout(layout);

    connect(m_prevButton, &DImageButton::clicked, [this]{
        setValue(m_value - 1);
    });
    connect(m_nextButton, &DImageButton::clicked, [this]{
        setValue(m_value + 1);
    });

    // 数字与翻月箭头随深浅主题初始化（运行时切换在 changeEvent 里再调）
    applyTheme();
}

void Spinner::changeEvent(QEvent *event)
{
    // 跟随系统调色板（深浅主题）变化刷新配色；做法同各对话框 setTheMe，
    // 监听 PaletteChange / ApplicationPaletteChange。
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        applyTheme();
    }
    QWidget::changeEvent(event);
}

void Spinner::applyTheme()
{
    const bool dark = DDE25::themeType() == 2;
    m_label->setStyleSheet(dark ? "QLabel { color: rgba(255, 255, 255, 0.9); }"
                                : "QLabel { color: #303030; }");
    // 重新套用翻月箭头图标（按主题选浅色/深色版）
    setPrevButtonDisabled(!m_prevButton->isEnabled());
    setNextButtonDisabled(!m_nextButton->isEnabled());
}

void Spinner::setRange(int min, int max)
{
    m_min = min;
    m_max = max;
}

int Spinner::value() const
{
    return m_value;
}

void Spinner::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        m_label->setText(QString::number(m_value));

        setPrevButtonDisabled(value <= m_min);
        setNextButtonDisabled(value >= m_max);

        emit valueChanged(value);
    }
}

void Spinner::setPrevButtonDisabled(bool disabled) const
{
    m_prevButton->setDisabled(disabled);
    if (disabled) {
        m_prevButton->setHoverPic(":/resources/icon/previous_disabled.svg");
        m_prevButton->setNormalPic(":/resources/icon/previous_disabled.svg");
        m_prevButton->setPressPic(":/resources/icon/previous_disabled.svg");
    } else if (DDE25::themeType() == 2) {
        // 深色主题用浅色描边的箭头（previous_nav_dark.svg）
        m_prevButton->setHoverPic(":/resources/icon/previous_nav_dark.svg");
        m_prevButton->setNormalPic(":/resources/icon/previous_nav_dark.svg");
        m_prevButton->setPressPic(":/resources/icon/previous_nav_dark.svg");
    } else {
        m_prevButton->setHoverPic(":/resources/icon/previous_hover.svg");
        m_prevButton->setNormalPic(":/resources/icon/previous_normal.svg");
        m_prevButton->setPressPic(":/resources/icon/previous_press.svg");
    }
}

void Spinner::setNextButtonDisabled(bool disabled) const
{
    m_nextButton->setDisabled(disabled);
    if (disabled) {
        m_nextButton->setHoverPic(":/resources/icon/next_disabled.svg");
        m_nextButton->setNormalPic(":/resources/icon/next_disabled.svg");
        m_nextButton->setPressPic(":/resources/icon/next_disabled.svg");
    } else if (DDE25::themeType() == 2) {
        // 深色主题用浅色描边的箭头（next_nav_dark.svg）
        m_nextButton->setHoverPic(":/resources/icon/next_nav_dark.svg");
        m_nextButton->setNormalPic(":/resources/icon/next_nav_dark.svg");
        m_nextButton->setPressPic(":/resources/icon/next_nav_dark.svg");
    } else {
        m_nextButton->setHoverPic(":/resources/icon/next_hover.svg");
        m_nextButton->setNormalPic(":/resources/icon/next_normal.svg");
        m_nextButton->setPressPic(":/resources/icon/next_press.svg");
    }
}
