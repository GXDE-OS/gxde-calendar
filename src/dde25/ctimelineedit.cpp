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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/ctimelineedit.*）。
 */

#include "ctimelineedit.h"

#include <QLineEdit>

CTimeLineEdit::CTimeLineEdit(int id, QWidget *parent)
    : DSpinBox(parent)
    , m_id(id)
{
    initView();
    connect(this, &CTimeLineEdit::editingFinished, this, &CTimeLineEdit::slotEditingFinished);
    connect(this->lineEdit(), &QLineEdit::textEdited, this, &CTimeLineEdit::slotTextEdited);
}

void CTimeLineEdit::initView()
{
    // 参考实现在这里调 setEnabledEmbedStyle(true) 打开嵌入式样式，
    // dtk2widget 的 DSpinBox 没有该接口，样式相关的调用一并去掉
}

/**
 * @brief CTimeLineEdit::setRange   设置数字显示范围
 * @param min 最小大小
 * @param max 最大大小
 */
void CTimeLineEdit::setRange(int min, int max)
{
    DSpinBox::setRange(min, max);
}

/**
 * @brief CTimeLineEdit::setStepEnabled     设置步状态
 * @param enable 状态
 */
void CTimeLineEdit::setStepEnabled(CTimeLineEdit::StepEnabled enable)
{
    m_stepEnable = enable;
}

/**
 * @brief CTimeLineEdit::setNum     设置显示的值
 * @param num 待显示的值
 */
void CTimeLineEdit::setNum(int num)
{
    m_num = num;
    m_num = m_num > minimum() ? m_num : minimum();
    m_num = m_num < maximum() ? m_num : maximum();
    setValue(num);
    emit signalNumChange(m_id, m_num);
}

/**
 * @brief CTimeLineEdit::setNum     设置显示的值
 * @param num 待显示的数字
 * @param canCarry true: 可以跳转， false: 不可以跳转
 */
void CTimeLineEdit::setNum(int num, bool canCarry)
{
    if (!canCarry) {
        setNum(num);
        return;
    }

    //若没有超过限制范围则不必发送时间跳转事件
    if (num >= minimum() && num <= maximum()) {
        m_num = num;
        setValue(m_num);
        emit signalNumChange(m_id, m_num);
        return;
    }

    //发送时间跳转信号， num-m_num: 时间差
    emit signalDateJump(m_id, num - m_num);
}

void CTimeLineEdit::slotEditingFinished()
{
    setNum(value());
}

void CTimeLineEdit::slotTextEdited(const QString &text)
{
    //过滤掉非数字字符
    QString value = "";
    for (QChar c : text) {
        if ('0' <= c && c <= '9') {
            value.append(c);
        }
    }
    //借用字符串转整数策略使其剩余字符都符合数字规范，包括无前置0等
    int v = value.toInt();

    //保存光标位置，因为存在删减字符，且被删减的字符只能是在首位，
    //因此不直接记录光标绝对位置，而是保存当前光标位置与最后位置的相对位置
    int len = text.length() - lineEdit()->cursorPosition();
    lineEdit()->setText(v == 0 ? "" : QString::number(v));
    //恢复光标位置
    lineEdit()->setCursorPosition(lineEdit()->text().length() - len);
}

/**
 * @brief CTimeLineEdit::stepEnabled
 * 因考虑到数字可以进位和退位，不存在真实的数字变化限制，因此返回up和down状态都可用
 * @return
 */
CTimeLineEdit::StepEnabled CTimeLineEdit::stepEnabled() const
{
    return m_stepEnable;
}

/**
 * @brief CTimeLineEdit::stepBy
 * 捕捉步长跳转事件，进行对数字变化的自定义
 * @param steps 步长（数字变化后应为的值与变化前的值的差值）
 */
void CTimeLineEdit::stepBy(int steps)
{
    setNum(value() + steps, true);
    //因为已自定义处理步长，因此再次调用父类的方法实现默认效果，并将其步长传入0
    DSpinBox::stepBy(0);
}
