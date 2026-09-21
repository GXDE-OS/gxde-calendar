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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/colorWidget/colorpickerWidget.*）。
 *
 * 差异：
 * 1. DTK2Widget 没有 DAbstractDialog 的转发头，改 include <dabstractdialog.h>；
 * 2. DTK2Widget 没有 DVerticalLine，分隔线换成 DSeparatorVertical；
 * 3. DTK2Widget 的 DLineEdit 直接继承 QLineEdit（没有 lineEdit() 取内部的写法），
 *    所以校验器、清空按钮都直接装在 m_colHexLineEdit 上；
 * 4. 去掉 tabletconfig.h / commondef.h（日志宏）。
 */

#ifndef COLORPICKERWIDGET_H
#define COLORPICKERWIDGET_H

#include "colorlabel.h"
#include "colorslider.h"

#include <dabstractdialog.h>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>

#include <QLabel>

DWIDGET_USE_NAMESPACE

class CColorPickerWidget : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit CColorPickerWidget(QWidget *parent = nullptr);
    ~CColorPickerWidget();

    QColor getSelectedColor();

private:
    void initUI();
    void setColorHexLineEdit();
    void setLabelText();

public slots:

    /**
     * @brief slotUpdateColor　画板颜色更新事件
     */
    void slotUpdateColor(const QColor &color = QColor());
    /**
     * @brief slotHexLineEditChange
     * 颜色输入框文本更改事件
     * @param text
     */
    void slotHexLineEditChange(const QString &text);

    void slotCancelBtnClicked();
    void slotEnterBtnClicked();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *e) override;
private:
    ColorLabel *m_colorLabel;
    ColorSlider *m_colorSlider;

    DLineEdit *m_colHexLineEdit;
    DLabel *m_wordLabel;
    DPushButton *m_cancelBtn;
    DPushButton *m_enterBtn;
    QString     m_strColorLabel;
    QColor curColor;
};

#endif // COLORPICKERWIDGET_H
