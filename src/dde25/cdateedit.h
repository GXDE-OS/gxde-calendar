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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cdateedit.*）。
 *
 * 差异：农历信息从 LunarManager 的 DBus 调用改成 DDE25::LunarCache，
 * getLunarName() 与参考实现一致，仍取「农历月名 + 农历日名」。
 */

#ifndef CDATEEDIT_H
#define CDATEEDIT_H

#include <QDateEdit>
#include <QTextLayout>

class CDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    explicit CDateEdit(QWidget *parent = nullptr);
    /**
     * @brief setData
     * 设置显示时间（覆盖父类方法）
     */
    void setDate(QDate);
    /**
     * @brief setDisplayFormat
     * 设置时间显示格式（覆盖父类方法）
     * @param format 时间格式
     */
    void setDisplayFormat(QString format);
    /**
     * @brief displayFormat
     * 获取当前时间格式（覆盖父类方法）
     * @return
     */
    QString displayFormat();
    /**
     * @brief setLunarCalendarStatus
     * 设置农历状态
     * @param status true：显示农历 false：不显示农历
     */
    void setLunarCalendarStatus(bool status);
    /**
     * @brief setLunarTextFormat
     * 设置农历文本样式
     * @param format
     */
    void setLunarTextFormat(QTextCharFormat format);
    /**
     * @brief getsetLunarTextFormat
     * 获取农历文本样式
     * @return
     */
    QTextCharFormat getsetLunarTextFormat();
    /**
     * @brief setCalendarPopup
     * 是否弹出日历选择界面
     * @param enable
     */
    void setCalendarPopup(bool enable);

    void setEditCursorPos(int pos);

signals:
protected:

    void changeEvent(QEvent *e) override;

private slots:

    //连接时间改变信号，填充农历信息
    void slotDateEidtInfo(const QDate &date);
    //连接输入框文本改变信号，刷新文本样式
    void slotRefreshLineEditTextFormat(const QString &);
    //连接光标位置改变信号，限制光标位置
    void slotCursorPositionChanged(int, int);
    //连接文本选择改变信号，处理键盘全选时情况
    void slotSelectionChanged();

private:
    //获取农历信息
    QString getLunarName(const QDate &);
    //设置文本样式
    void setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &formats);
    //更新日历显示类型
    void updateCalendarWidget();
    bool showGongli();
private:
    QString m_format = "";      //时间格式化格式
    bool m_showLunarCalendar = false;    //农历显示状态 true：显示 false：不显示
    QString m_lunarName = "";   //农历字符串
    QTextCharFormat m_lunarTextFormat;  //农历文本样式
    QString m_strCurrrentDate = "";
};

#endif // CDATEEDIT_H
