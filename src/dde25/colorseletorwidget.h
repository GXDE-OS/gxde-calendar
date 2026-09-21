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
 *
 * 差异（本项目单进程、无账户，砍掉 CalDAV 分支）：
 * 1. resetColorButton() 不再吃 AccountItem，系统色直接取
 *    CalendarService::getSysColors()，因此 m_allowCustomColor 恒为 true，
 *    这个开关连同 CalDAV 固定色板一起删掉；
 * 2. “上次用的颜色”从 CConfigSettings 改成 QSettings（跟本项目其它地方一致）；
 * 3. 加号图标 DTK2Widget 没有 DStyle，改用 QIcon::fromTheme("list-add")；
 * 4. 删掉 Qt5.11 的兼容槽（本项目只编 Qt6）。
 */

#ifndef COLORSELETOR_H
#define COLORSELETOR_H

#include "cradiobutton.h"
#include "dtypecolor.h"

#include <QButtonGroup>
#include <QColor>
#include <QHBoxLayout>
#include <QMap>
#include <QWidget>

class QPushButton;

//色彩控件选择类
class ColorSeletorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSeletorWidget(QWidget *parent = nullptr);

    /**
     * @brief reset
     * 重置
     */
    void reset();
    /**
     * @brief getSelectColor
     * @return 当前已选在的色彩
     */
    DTypeColor::Ptr getSelectedColorInfo();
    /**
     * @brief setUserColor 设置用户自定义的色彩
     */
    void setUserColor(const DTypeColor::Ptr &);
    /**
     * @brief setSelectedColorByIndex 设置选择的色彩控件
     * @param index 色彩控件位置
     */
    void setSelectedColorByIndex(int index);
    /**
     * @brief setSelectedColorById 设置选择的色彩控件
     * @param colorId 色彩id
     */
    void setSelectedColorById(int colorId);
    /**
     * @brief setSelectedColor 设置选择的色彩控件
     * @param color 色彩实例
     */
    void setSelectedColor(const DTypeColor &);
    /**
     * @brief resetColorButton
     * 重置色彩控件
     */
    void resetColorButton();

signals:
    //选择的色彩改变信号
    void signalColorChange(DTypeColor::Ptr);

public slots:
    //色彩控件点击信号
    void slotButtonClicked(int butId);
    //添加自定义色彩控件点击信号
    void slotAddColorButClicked();

private:
    void init();
    void initView();

    //添加色彩控件
    void addColor(const DTypeColor::Ptr &);

private:
    QMap<int, DTypeColor::Ptr> m_colorEntityMap; //所有色彩实体
    QHBoxLayout *m_colorLayout = nullptr;     //色彩控件布局类
    QPushButton *m_addColorButton = nullptr;  //自定义色彩按钮
    QButtonGroup *m_colorGroup = nullptr;     //所有色彩控件
    DTypeColor::Ptr m_colorInfo; //当前已选择的色彩
    CRadioButton *m_userColorBtn = nullptr; //用户自定义的色彩控件
    const int m_userColorBtnId = 999; //用户自定义的色彩控件id
};

#endif // COLORSELETOR_H
