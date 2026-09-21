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
 * 移植自 dde-calendar（src/calendar-client/src/dialog/schedulectrldlg.*）。
 * 差异见头文件注释。
 */

#include "schedulectrldlg.h"

#include "constants.h"
#include "dde25common.h"

#include <DPushButton>

#include <QTimer>

CScheduleCtrlDlg::CScheduleCtrlDlg(QWidget *parent)
    : DCalendarDDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
    setTheMe(DDE25::themeType());
    resize(380, 260);
}

void CScheduleCtrlDlg::initUI()
{
    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);
    //参考实现是 CDynamicIcon（一张随日期变化的日程图标），这里用应用自己的图标
    setIcon(QIcon(":/resources/icon/gxde-calendar.svg"), QSize(32, 32));

    m_mainBoxLayout = new QVBoxLayout();
    m_mainBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_mainBoxLayout->setSpacing(0);

    m_firstLabel = new QLabel();
    m_firstLabel->setAlignment(Qt::AlignCenter);
    m_firstLabel->setFixedWidth(350);
    m_firstLabel->setWordWrap(true);
    labelF.setWeight(QFont::DemiBold);
    //参考实现绑的是 DFontSizeManager::T6，DTK2Widget 没有这个类，
    //按本项目既有做法（T6 -> 12px）直接设像素字号
    labelF.setPixelSize(DDECalendar::FontSizeTwelve);
    m_firstLabel->setForegroundRole(QPalette::WindowText);
    m_firstLabel->setFont(labelF);
    m_mainBoxLayout->addWidget(m_firstLabel);

    m_seconLabel = new QLabel();
    m_seconLabel->setFixedWidth(350);
    m_seconLabel->setAlignment(Qt::AlignCenter);

    labelT.setWeight(QFont::Normal);
    labelT.setPixelSize(DDECalendar::FontSizeTwelve);
    m_seconLabel->setForegroundRole(QPalette::WindowText);
    m_seconLabel->setFont(labelT);
    m_seconLabel->setWordWrap(true);
    m_mainBoxLayout->addSpacing(3);
    m_mainBoxLayout->addWidget(m_seconLabel);
    gwi = new DFrame(this);
    gwi->setContentsMargins(0, 0, 0, 0);
    gwi->setLayout(m_mainBoxLayout);
    QPalette anipa = gwi->palette();
    QColor color = "#F8F8F8";
    color.setAlphaF(0.0);
    anipa.setColor(QPalette::Window, color);
    gwi->setAutoFillBackground(true);
    gwi->setPalette(anipa);
    gwi->setBackgroundRole(QPalette::Window);
    addContent(gwi, Qt::AlignCenter);
}

void CScheduleCtrlDlg::initConnection()
{
    connect(this, &DDialog::buttonClicked, this, &CScheduleCtrlDlg::buttonJudge); //连接信号和槽
}

void CScheduleCtrlDlg::setTheMe(const int type)
{
    //标题文字颜色
    QColor titleColor;
    //提示内容文字颜色
    QColor contentColor;
    if (type == 2) {
        titleColor = "#FFFFFF";
        titleColor.setAlphaF(0.9);
        contentColor = "#FFFFFF";
        contentColor.setAlphaF(0.7);
    } else {
        titleColor = "#000000";
        titleColor.setAlphaF(0.9);
        contentColor = "#000000";
        contentColor.setAlphaF(0.7);
    }
    setPaletteTextColor(m_firstLabel, titleColor);
    setPaletteTextColor(m_seconLabel, contentColor);
}

void CScheduleCtrlDlg::setPaletteTextColor(QWidget *widget, QColor textColor)
{
    //如果为空指针则退出
    if (nullptr == widget) {
        return;
    }
    QPalette palette = widget->palette();
    //设置文字显示颜色
    palette.setColor(QPalette::WindowText, textColor);
    widget->setPalette(palette);
}

void CScheduleCtrlDlg::changeEvent(QEvent *event)
{
    //参考实现连的是 DGuiApplicationHelper::themeTypeChanged，
    //DTK2Widget 下没有这个信号，改成跟着调色板变化重新取色
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        setTheMe(DDE25::themeType());
    }

    DCalendarDDialog::changeEvent(event);

    //initUI 之前（构造过程中 setPalette 引起的）变化事件里控件还没建出来
    if (m_firstLabel == nullptr || m_seconLabel == nullptr || gwi == nullptr) {
        return;
    }

    QFont font;
    QFontMetrics font_button(font);
    QFontMetrics font_firstLabel(font);
    QFontMetrics font_seconLabel(font);
    int height_firstLabel = (font_firstLabel.horizontalAdvance(m_firstLabel->text()) / 300 + 1) * font_firstLabel.height();
    int height_seconLabel = (font_seconLabel.horizontalAdvance(m_seconLabel->text()) / 300 + 1) * font_seconLabel.height();

    for (int i = 0; i < buttonCount(); i++) {
        QAbstractButton *button = getButton(i);
        QString str = str_btName.at(i);
        QString text_button = font_button.elidedText(str, Qt::ElideRight, 112);

        if (str.size() == 2) {
            button->setText(QString().append(str.at(0)).append(QChar::Nbsp).append(str.at(1)));
        } else {
            button->setText(text_button);
        }
    }
    // 在changeEvent里使用setFixedHeight会导致弹出的确认对话框会显示在左上角
    // 推测是窗口的问题, 先通过延迟在应用层临时解决
    QTimer::singleShot(10, this, [this, height_firstLabel, height_seconLabel] {
        setFixedHeight(36 + 48 + height_firstLabel + height_seconLabel + 30);
        gwi->setFixedHeight(height_firstLabel + height_seconLabel);
    });
}

void CScheduleCtrlDlg::buttonJudge(int id)
{
    m_id = id;
    accept();
}

QAbstractButton *CScheduleCtrlDlg::addPushButton(QString btName, bool type)
{
    addButton(btName, false, DDialog::ButtonNormal);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *button = getButton(button_index);

    if (type) {
        button->setFixedHeight(36);
        button->setFixedWidth(165);
    } else {
        button->setFixedHeight(36);
        button->setFixedWidth(129);
    }

    button->setToolTip(btName);
    str_btName.append(btName);

    return button;
}

QAbstractButton *CScheduleCtrlDlg::addsuggestButton(QString btName, bool type)
{
    addButton(btName, false, DDialog::ButtonRecommend);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *suggestButton = getButton(button_index);

    if (type) {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(165);
    } else {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(129);
    }

    suggestButton->setToolTip(btName);
    str_btName.append(btName);

    return suggestButton;
}

QAbstractButton *CScheduleCtrlDlg::addWaringButton(QString btName, bool type)
{
    addButton(btName, false, DDialog::ButtonWarning);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *suggestButton = getButton(button_index);

    if (type) {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(165);
    } else {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(129);
    }

    suggestButton->setToolTip(btName);
    str_btName.append(btName);

    return suggestButton;
}

void CScheduleCtrlDlg::setText(QString str)
{
    m_firstLabel->setText(str);
    m_firstLabel->setToolTip(str);
}

void CScheduleCtrlDlg::setInformativeText(QString str)
{
    m_seconLabel->setText(str);
    m_seconLabel->setToolTip(str);
}

int CScheduleCtrlDlg::clickButton()
{
    if (m_id < 0 || m_id > buttonCount() - 1) return buttonCount();
    return  m_id;
}
