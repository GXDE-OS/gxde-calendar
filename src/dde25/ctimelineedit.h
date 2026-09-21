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
 *
 * 差异：参考实现构造时调 DSpinBox::setEnabledEmbedStyle(true) 打开嵌入式样式，
 * dtk2widget 的 DSpinBox 没有这个接口（见 dtk2widget-qt6 的 dspinbox.h），
 * 去掉不影响数值逻辑，只影响上下按钮的绘制样式。
 */

#ifndef CTIMELINEEDIT_H
#define CTIMELINEEDIT_H

#include <dspinbox.h>

DWIDGET_USE_NAMESPACE

//时间数字编辑器
class CTimeLineEdit : public DSpinBox
{
    Q_OBJECT
public:
    explicit CTimeLineEdit(int id = 0, QWidget *parent = nullptr);
    ~CTimeLineEdit() override = default;

    //设置控件数字，值将被限制在临界值范围内
    void setNum(int num);
    //设置控件数字，如果canCarry为true且超过临界值将进行自动跳转时间
    void setNum(int num, bool canCarry);
    //设置数字显示范围
    void setRange(int min, int max);
    //设置步状态
    void setStepEnabled(CTimeLineEdit::StepEnabled);

signals:
    //数字改变信号
    void signalNumChange(int id, int num);
    //时间跳转信号
    void signalDateJump(int id, int num);

public slots:
    //编辑完成事件
    void slotEditingFinished();
    //编辑框编辑事件
    void slotTextEdited(const QString &value);

protected:
    //重写步状态
    StepEnabled stepEnabled() const override;
    //重写步事件
    void stepBy(int steps) override;

private:
    void initView();

private:
    int m_num = 0;      //当前显示的数字
    const int m_id = 0; //控件id
    CTimeLineEdit::StepEnabled m_stepEnable = CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled;
};

#endif // CTIMELINEEDIT_H
