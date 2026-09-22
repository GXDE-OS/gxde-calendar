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
 * 移植自 dde-calendar（src/calendar-client/src/dialog/scheduledlg.*）。
 * 界面、校验规则、保存映射全部保留，砍掉的是账户体系：
 *
 * 1. 去掉「日历账户」一行以及 m_accountComBox/AccountItem/DAccount，整个
 *    slotAccountUpdate/slotAccoutBoxActivated/slotAccountStateChange/signalLogout
 *    随之删除。所属日历只由 JobTypeComboBox 一个下拉框承担；
 * 2. CScheduleOperation（走 DBus 到 service 进程）换成进程内的
 *    CalendarService::createSchedule/updateSchedule；
 * 3. 因此 canWriteCurrentCalDavCollection() 与 m_bCanCreateType 的异步回调
 *    一并去掉，createSchedule 与 selectScheduleType 变成同步返回；
 * 4. DTK2Widget 没有 DComboBox/DDateEdit/DRadioButton/DWidget，分别换成
 *    QComboBox/CDateEdit/QRadioButton/QWidget；DFontSizeManager 没有，
 *    原来绑的 T6 直接不设，用父控件字体（参考实现本来也都另外 setFont 过）；
 * 5. DPalette::Text -> QPalette::Text，CConfigSettings -> QSettings，
 *    CDynamicIcon -> 应用自己的图标；
 * 6. DLineEdit 在 DTK2Widget 下直接继承 QLineEdit，没有 lineEdit()，
 *    「结束与次数」的校验器直接挂在它自己身上；
 * 7. 去掉 Qt5.11 兼容分支（Qt6 下 textActivated/idClicked 都在）。
 * 8. createSchedule() 里的「重复结束」只在「重复」下拉不是「从不」时才写，参考
 *    实现是无条件写的；initRmindRpeatUI() 回填时也先看有没有 RRULE。原因见
 *    createSchedule() 中的注释：KCalendarCore 的 setDuration()/setEndDateTime()
 *    在没有 RRULE 时会凭空补一条空规则，序列化成 "RRULE:ERROR: No Value"。
 */

#ifndef SCHEDULEDLG_H
#define SCHEDULEDLG_H

#include "dschedule.h"

#include "cdateedit.h"
#include "colorseletorwidget.h"
#include "dcalendarddialog.h"
#include "jobtypecombobox.h"

//dtk2 的 dcheckbox.h 只是个占位头（`class DCheckBox: QCheckBox` 私有继承、
//只有一个无参构造），用不了，全天勾选框直接用 QCheckBox
#include <DLineEdit>

//dtk2 只装了小写的 dtextedit.h，没有 <DTextEdit> 包装头
#include <dtextedit.h>
//dtk2 的 <DFrame> 包装头和 dframe.h 共用 DFRAME_H 宏，include 等于没做，
//直接 include 实体头（详见 daymonthview.h 里的记录）
#include <dframe.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>

DWIDGET_USE_NAMESPACE

class CTimeEdit;
class CScheduleDlg : public DCalendarDDialog
{
    Q_OBJECT
public:

    enum ButtonId {
        RadioSolarId = 0, //公历选择按钮id
        RadioLunarId      //农历选择按钮id
    };

    CScheduleDlg(int type, QWidget *parent = nullptr, const bool isAllDay = true);
    ~CScheduleDlg() override;
    void setData(const DSchedule::Ptr &info);
    void setDate(const QDateTime &date);
    void setAllDay(bool flag);
private:
    //确定按钮处理
    bool clickOkBtn();
    //选择日程类型
    bool selectScheduleType();
    //创建日程
    bool createSchedule(const QString &scheduleTypeId);

    /**
     * @brief updateEndTimeListAndTimeDiff      更新结束时间（time），结束时间下拉列表和开始时间和结束时间差
     * @param begin                             开始时间（DateTime）
     * @param end                               结束时间 (DateTime）
     */
    void updateEndTimeListAndTimeDiff(const QDateTime &begin, const QDateTime &end);

    /**
     * @brief updateEndTimeList             更新结束时间下拉列表
     * @param begin                         开始时间(time)
     * @param isShowTimeInterval            是否显示时间偏移
     */
    void updateEndTimeList(const QTime &begin, bool isShowTimeInterval);
signals:
    void signalScheduleUpdate(int id = 0);
public slots:
    /**
     * @brief 开始时间改变
     */
    void slotBeginTimeChange();

    /**
     * @brief 结束时间改变
     */
    void slotEndTimeChange();

    /**
     * @brief 结束日期改变
     */
    void slotEndDateChange(const QDate &date);
    /**
     * @brief slotRadioBtnClicked
     * 选择按钮控件点击时间
     * @param btnId 控件id
     */
    void slotRadioBtnClicked(int btnId);
    /**
     * @brief slotBtnAddItemClicked
     * 添加类型事件
     */
    void slotBtnAddItemClicked();
    /**
     * @brief slotTypeEditTextChanged
     * 类型下拉选择框文本改变事件
     */
    void slotTypeEditTextChanged(const QString &);
    void slotTypeRpeatactivated(int index);
    //对话框按钮点击处理
    void slotBtClick(int buttonIndex, const QString &buttonName);
    void slotTextChange();
    void slotendrepeatTextchange();
    void slotBDateEidtInfo(const QDate &date);
    void slotallDayStateChanged(int state);
    void slotbRpeatactivated(int index);
    void sloteRpeatactivated(int index);
    void slotJobComboBoxEditingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *pEvent) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void updateDateTimeFormat() override;

private:
    void initUI();
    void resetColor();
    void initConnection();
    void initDateEdit();
    void initJobTypeComboBox();
    void initRmindRpeatUI();
    /**
     * @brief setTheMe  根据主题type设置颜色
     * @param type  主题type
     */
    void setTheMe(const int type);
    //设置tab顺序
    void setTabFouseOrder();

    /**
     * @brief updateIsOneMoreDay        更新开始时间与结束时间标识是否超过一天
     * @param begin                     开始时间(datetime)
     * @param end                       结束时间(datetime)
     */
    void updateIsOneMoreDay(const QDateTime &begin, const QDateTime &end);
    /**
     * @brief updateRepeatCombox
     * 更新重复下拉列表
     * @param isLunar   是否是农历 true：是 false：不是
     */
    void updateRepeatCombox(bool isLunar = false);
    /**
     * @brief isShowLunar
     * 系统语言检查，根据语言类型判断是否显示农历信息
     */
    bool isShowLunar();

    /**
     * @brief setShowState      设置显示状态
     * @param jobIsLunar        日程是否为农历
     */
    void setShowState(bool jobIsLunar);

    void setWidgetEnabled(bool isEnabled);

    void resize();

    /**
     * @brief setOkBtnEnabled   根据选项设置保存按钮是否有效
     */
    void setOkBtnEnabled();

private:
    QLabel *m_typeLabel = nullptr;
    JobTypeComboBox *m_typeComBox = nullptr;
    ColorSeletorWidget *m_colorSeletorWideget = nullptr; //颜色选择器
    QLabel *m_contentLabel = nullptr;
    DTextEdit *m_textEdit = nullptr;
    QLabel *m_beginTimeLabel = nullptr;
    CDateEdit *m_beginDateEdit = nullptr;
    CTimeEdit *m_beginTimeEdit = nullptr;
    QLabel *m_endTimeLabel = nullptr;
    CDateEdit *m_endDateEdit = nullptr;
    CTimeEdit *m_endTimeEdit = nullptr;

    QLabel *m_adllDayLabel = nullptr;
    QCheckBox *m_allDayCheckbox = nullptr;
    //提醒（m_remindSetLabel / m_rmindCombox）连同 DSchedule::AlarmType 一起删掉了：
    //本项目没有提醒引擎（没有 daemon，CalendarService::getRemindSchedule() 也没有
    //调用者），下拉框选出来的提醒从来不会响
    QLabel *m_beginrepeatLabel = nullptr;
    QComboBox *m_beginrepeatCombox = nullptr;
    QLabel *m_endrepeatLabel = nullptr;
    QComboBox *m_endrepeatCombox = nullptr;
    DLineEdit *m_endrepeattimes = nullptr;
    QLabel *m_endrepeattimesLabel = nullptr;
    QWidget *m_endrepeattimesWidget = nullptr;
    CDateEdit *m_endRepeatDate = nullptr;
    QWidget *m_endrepeatWidget = nullptr;
    DFrame *m_gwi = nullptr;
    QLabel *m_titleLabel = nullptr;
    QButtonGroup *m_calendarCategoryRadioGroup = nullptr;    //日历类别选择控件组
    QRadioButton *m_solarRadioBtn = nullptr;      //公历选择按钮
    QRadioButton *m_lunarRadioBtn = nullptr;        //农历选择按钮

    QString m_context;
    QString m_TypeContext;  //类型输入框上一次输入后的字符
    const bool m_createAllDay;
    bool m_setAccept {false}; //是否设置返回代码为Rejected
    qint64 m_timeDiff = 0;             //开始时间和结束时间差，不算日期
    bool m_isMoreThenOneDay = false;
private:
    //日程
    DSchedule::Ptr m_scheduleDataInfo;
    int m_type; // 1新建 0 编辑日程
    QDateTime m_currentDate;
    QDateTime m_EndDate;
    bool m_typeEditStatus = false; //日程类型编辑状态
    int m_prevCheckRadioID = -1; //上一次点击Radio的id编号
};

#endif // SCHEDULEDLG_H
