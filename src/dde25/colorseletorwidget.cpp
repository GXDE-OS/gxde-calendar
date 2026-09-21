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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/colorseletorwidget.*）。
 */

#include "colorseletorwidget.h"

#include "calendarservice.h"
#include "colorpickerwidget.h"

#include <QIcon>
#include <QPushButton>
#include <QSettings>

ColorSeletorWidget::ColorSeletorWidget(QWidget *parent) : QWidget(parent)
{
    init();
}

void ColorSeletorWidget::init()
{
    initView();
    m_colorGroup = new QButtonGroup(this);
    m_colorGroup->setObjectName("ColorGroup");
    m_colorGroup->setExclusive(true);
    connect(m_colorGroup, &QButtonGroup::idClicked, this, &ColorSeletorWidget::slotButtonClicked);

    m_colorInfo.reset(new DTypeColor());
}

void ColorSeletorWidget::resetColorButton()
{
    reset();

    //添加默认颜色控件
    DTypeColor::List colorList = CalendarService::instance()->getSysColors();
    for (DTypeColor::Ptr &var : colorList) {
        if (DTypeColor::PriSystem == var->privilege()) {
            addColor(var);
        }
    }

    //自定义控件单独添加
    m_colorGroup->addButton(m_userColorBtn, m_userColorBtnId);
    m_colorLayout->addWidget(m_userColorBtn);

    if (m_colorGroup->buttons().size() > 0) {
        m_colorGroup->buttons().at(0)->click();
    }
}

void ColorSeletorWidget::reset()
{
    //清空所有的色彩实体和控件
    m_colorEntityMap.clear();
    QList<QAbstractButton *> buttons = m_colorGroup->buttons();
    for (QAbstractButton *btn : buttons) {
        m_colorGroup->removeButton(btn);
        m_colorLayout->removeWidget(btn);
        //自定义控件内存不释放
        if (btn == m_userColorBtn) {
            continue;
        }
        delete btn;
        btn = nullptr;
    }
    if (nullptr == m_userColorBtn) {
        m_userColorBtn = new CRadioButton(this);

        m_userColorBtn->setObjectName("UserColorBtn");
        m_userColorBtn->setAccessibleName("UserColorBtn");
        m_userColorBtn->setFixedSize(18, 18);
    }
    m_userColorBtn->hide();
}

void ColorSeletorWidget::addColor(const DTypeColor::Ptr &cInfo)
{
    static int count = 0;   //静态变量，充当色彩控件id
    count++;
    m_colorEntityMap.insert(count, cInfo); //映射id与控件,从1开始
    CRadioButton *radio = new CRadioButton(this);
    radio->setColor(QColor(cInfo->colorCode())); //设置控件颜色
    radio->setFixedSize(18, 18);
    m_colorGroup->addButton(radio, count);
    m_colorLayout->addWidget(radio);
}

DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo()
{
    QSettings settings;
    if (m_colorInfo->privilege() == DTypeColor::PriSystem) {
        settings.setValue("LastSysColorTypeNo", m_colorInfo->colorID());
    } else if (!m_colorInfo->colorCode().isEmpty()) {
        settings.setValue("LastUserColor", m_colorInfo->colorCode());
        settings.setValue("LastSysColorTypeNo", "");
    }
    return m_colorInfo;
}

void ColorSeletorWidget::setSelectedColorByIndex(int index)
{
    if (index >= 0 && index < m_colorGroup->buttons().size()) {
        QAbstractButton *but = m_colorGroup->buttons().at(index);
        if (nullptr != but) {
            but->click();
        }
    }
}

void ColorSeletorWidget::setSelectedColorById(int colorId)
{
    //默认选择第一个
    if (colorId < 0) {
        if (m_colorGroup->buttons().size() > 0) {
            m_colorGroup->buttons().at(0)->click();
        }
        return;
    } else if (colorId == 9) {
        m_userColorBtn->click();
        return;
    }

    //系统颜色则向后移一位
    if (colorId == 8) {
        colorId = 0;
    } else {
        ++colorId;
    }
    if (m_colorGroup->buttons().size() > colorId) {
        m_colorGroup->buttons().at(colorId)->click();
    }
}

void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo)
{
    bool finding = false;
    auto iterator = m_colorEntityMap.begin();
    while (iterator != m_colorEntityMap.end()) {
        const bool sameColorID = !colorInfo.colorID().isEmpty()
            && iterator.value()->colorID() == colorInfo.colorID();
        const bool sameColorCode = !colorInfo.colorCode().isEmpty()
            && iterator.value()->colorCode().compare(colorInfo.colorCode(), Qt::CaseInsensitive) == 0;
        if (sameColorID || sameColorCode) {
            QAbstractButton *btn = m_colorGroup->button(iterator.key());
            if (btn) {
                btn->click();
                finding = true;
            }
            break;
        }
        iterator++;
    }
    if (!finding) {
        DTypeColor::Ptr ptr;
        ptr.reset(new DTypeColor(colorInfo));
        setUserColor(ptr);
    }
}

void ColorSeletorWidget::initView()
{
    m_colorLayout = new QHBoxLayout();

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(m_colorLayout);
    m_addColorButton = new QPushButton();
    m_addColorButton->setIcon(QIcon::fromTheme("list-add"));
    m_addColorButton->setFixedSize(18, 18);
    m_addColorButton->setIconSize(QSize(10, 10));
    hLayout->addWidget(m_addColorButton);
    hLayout->addStretch(1);

    m_colorLayout->setContentsMargins(0, 0, 0, 0);
    m_colorLayout->setSpacing(3);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(3);

    this->setLayout(hLayout);

    connect(m_addColorButton, &QPushButton::clicked, this, &ColorSeletorWidget::slotAddColorButClicked);
}

void ColorSeletorWidget::slotButtonClicked(int butId)
{
    auto it = m_colorEntityMap.find(butId);
    if (m_colorEntityMap.end() == it) {
        return;
    }
    DTypeColor::Ptr info = it.value();
    if (info->colorCode() != m_colorInfo->colorCode()
        || info->colorID() != m_colorInfo->colorID()) {
        m_colorInfo = info;
        emit signalColorChange(info);
    }
}

void ColorSeletorWidget::slotAddColorButClicked()
{
    CColorPickerWidget colorPicker;

    if (colorPicker.exec()) {
        DTypeColor::Ptr typeColor;
        typeColor.reset(new DTypeColor());
        typeColor->setColorCode(colorPicker.getSelectedColor().name());
        typeColor->setPrivilege(DTypeColor::PriUser);
        setUserColor(typeColor);
        m_userColorBtn->click();
    }
}

void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo)
{
    if (nullptr == m_userColorBtn || DTypeColor::PriUser != colorInfo->privilege()) {
        return;
    }
    if (!m_userColorBtn->isVisible()) {
        m_userColorBtn->show();
    }
    m_userColorBtn->setColor(colorInfo->colorCode());
    m_colorEntityMap[m_userColorBtnId] = colorInfo;
    m_userColorBtn->click();
}
