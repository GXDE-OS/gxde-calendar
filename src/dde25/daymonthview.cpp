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
 * Origin copyright bearer: 2015 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/widget/dayWidget/daymonthview.*）。
 */

#include "daymonthview.h"

#include "constants.h"
#include "cweekwidget.h"
#include "dde25common.h"

#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QWheelEvent>

CDayMonthView::CDayMonthView(QWidget *parent)
    : CustomFrame(parent)
{
    m_weeklist.append(tr("Monday"));
    m_weeklist.append(tr("Tuesday"));
    m_weeklist.append(tr("Wednesday"));
    m_weeklist.append(tr("Thursday"));
    m_weeklist.append(tr("Friday"));
    m_weeklist.append(tr("Saturday"));
    m_weeklist.append(tr("Sunday"));
    initUI();
    initConnection();
}

CDayMonthView::~CDayMonthView() = default;

void CDayMonthView::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
{
    m_selectDate = selectDate;
    m_currentDate = currentDate;
    m_dayMonthWidget->setShowDate(showDate, selectDate, currentDate);
    updateDateShow();
    update();
}

void CDayMonthView::setLunarVisible(bool visible)
{
    m_huanglistate = visible;
    m_currentLuna->setVisible(visible);
    update();
}

void CDayMonthView::setFirstWeekDay(Qt::DayOfWeek firstDay)
{
    m_firstWeekDay = firstDay;
    m_weekWidget->setFirstDay(firstDay);
}

void CDayMonthView::setTheMe(int type)
{
    const QColor todayColor = DDE25::systemActiveColor();
    m_themetype = type;
    m_prevButton->setIcon(DDE25::navArrowIcon(false, type));
    m_nextButton->setIcon(DDE25::navArrowIcon(true, type));
    m_dayMonthWidget->setTheMe(type);
    if (type == 0 || type == 1) {
        QPalette aniPa = palette();
        const QColor tbColor = "#FFFFFF";
        aniPa.setColor(QPalette::Window, tbColor);
        setPalette(aniPa);
        setBackgroundRole(QPalette::Window);
        setBColor(tbColor);

        m_currentMouth->setTextColor(QColor("#3B3B3B"));
        m_currentDay->setTextColor(todayColor);
        m_currentWeek->setTextColor(QColor("#414D68"));
        m_currentLuna->setTextColor(QColor("#414D68"));
        m_currentYear->setTextColor(QColor("#414D68"));

        m_backgroundCircleColor = "#0081FF";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;
        m_splitline->setStyleSheet("background-color: rgba(0, 0, 0, 0.1);");
    } else if (type == 2) {
        QPalette aniPa = palette();
        const QColor tbColor = "#282828";
        aniPa.setColor(QPalette::Window, tbColor);
        setPalette(aniPa);
        setBackgroundRole(QPalette::Window);
        setBColor(tbColor);

        m_currentMouth->setTextColor(QColor("#C0C6D4"));
        m_currentDay->setTextColor(todayColor);
        m_currentWeek->setTextColor(QColor("#C0C6D4"));
        m_currentLuna->setTextColor(QColor("#C0C6D4"));
        m_currentYear->setTextColor(QColor("#C0C6D4"));

        m_backgroundCircleColor = "#0059D2";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;
        m_splitline->setStyleSheet("background-color: rgba(255, 255, 255, 0.1);");
    }
    update();
}

void CDayMonthView::setLunarInfo(const CaLunarDayInfo &lunarInfo)
{
    m_lunarInfo = lunarInfo;
    updateDateLunarDay();
}

void CDayMonthView::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
{
    m_dayMonthWidget->setHasScheduleFlag(hasScheduleFlag);
}

void CDayMonthView::initUI()
{
    m_today = new QPushButton(tr("Today"), this);
    m_today->setObjectName("DayMonthViewTodayButton");
    m_today->setAccessibleName("DayMonthViewTodayButton");
    // 参考实现固定 80×36。文案固定为英文 Today 后 80px 足够，这里仍用最小宽度，
    // 避免不同字体下裁字（标题行有富余）。
    m_today->setMinimumSize(80, DDEDayCalendar::D_MLabelHeight);
    m_today->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    QFont todayfont;
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);

    m_prevButton = new QToolButton(this);
    m_prevButton->setObjectName("DayMonthPrevButton");
    m_prevButton->setAccessibleName("DayMonthPrevButton");
    m_prevButton->setAutoRaise(true);
    m_prevButton->setIcon(DDE25::navArrowIcon(false, m_themetype));
    m_prevButton->setIconSize(QSize(16, 16));
    m_prevButton->setFixedSize(36, 36);

    m_nextButton = new QToolButton(this);
    m_nextButton->setObjectName("DayMonthNextButton");
    m_nextButton->setAccessibleName("DayMonthNextButton");
    m_nextButton->setAutoRaise(true);
    m_nextButton->setIcon(DDE25::navArrowIcon(true, m_themetype));
    m_nextButton->setIconSize(QSize(16, 16));
    m_nextButton->setFixedSize(36, 36);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 10, 3);

    m_currentMouth = new CustomFrame(this);
    m_currentMouth->setFixedSize(74, DDEDayCalendar::D_MLabelHeight);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_currentMouth->setTextFont(mlabelF);
    m_currentMouth->setTextAlign(Qt::AlignCenter);
    titleLayout->addWidget(m_prevButton);
    titleLayout->addWidget(m_currentMouth);
    titleLayout->addWidget(m_nextButton);
    titleLayout->addStretch();
    titleLayout->addWidget(m_today, 0, Qt::AlignRight);

    // 上半部分
    m_upLayout = new QVBoxLayout;
    m_upLayout->setContentsMargins(0, 0, 0, 0);
    m_upLayout->setSpacing(0);
    m_upLayout->setContentsMargins(22, 9, 0, 7);
    m_upLayout->addLayout(titleLayout);

    m_weekWidget = new CWeekWidget(this);
    m_weekWidget->setObjectName("WeekWidget");
    m_weekWidget->setAccessibleName("WeekWidget");
    m_weekWidget->setMaximumHeight(40);
    m_dayMonthWidget = new CDayMonthWidget(this);
    m_upLayout->addWidget(m_weekWidget, 1);
    m_upLayout->addWidget(m_dayMonthWidget, 6);

    // 中间部分
    QVBoxLayout *midLayout = new QVBoxLayout;
    midLayout->setContentsMargins(0, 0, 0, 0);
    midLayout->setSpacing(0);
    midLayout->setContentsMargins(0, 0, 0, 20);

    m_currentDay = new CustomFrame(this);
    m_currentDay->setFixedHeight(DDEDayCalendar::DDLabelHeight);
    m_currentDay->setTextAlign(Qt::AlignCenter);
    QFont dayLabelF;
    dayLabelF.setWeight(QFont::Medium);
    dayLabelF.setPixelSize(DDECalendar::FontSizeOneHundred);
    m_currentDay->setTextFont(dayLabelF);
    midLayout->addWidget(m_currentDay);

    m_currentWeek = new CustomFrame(this);
    m_currentWeek->setFixedHeight(DDEDayCalendar::DWLabelHeight);
    m_currentWeek->setTextAlign(Qt::AlignCenter);
    QFont wLabelF;
    wLabelF.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentWeek->setTextFont(wLabelF);
    midLayout->addWidget(m_currentWeek);
    midLayout->addSpacing(2);

    m_currentYear = new CustomFrame(this);
    m_currentYear->setFixedHeight(DDEDayCalendar::DWLabelHeight);
    m_currentYear->setTextAlign(Qt::AlignCenter);
    m_currentYear->setTextFont(wLabelF);
    midLayout->addWidget(m_currentYear);
    midLayout->addSpacing(2);

    m_currentLuna = new CustomFrame(this);
    m_currentLuna->setFixedHeight(DDEDayCalendar::DHuangLiInfoLabelHeight);
    m_currentLuna->setTextAlign(Qt::AlignCenter);
    QFont hLabelF;
    hLabelF.setPixelSize(DDECalendar::FontSizeTwelve);
    m_currentLuna->setTextFont(hLabelF);
    midLayout->addWidget(m_currentLuna);

    m_hhLayout = new QVBoxLayout;
    m_hhLayout->setContentsMargins(0, 0, 0, 0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->addLayout(m_upLayout, 6);
    m_hhLayout->addLayout(midLayout);

    m_splitline = new QFrame(this);
    m_splitline->setFrameShape(QFrame::NoFrame);
    m_splitline->setFixedHeight(1);
    m_splitline->setVisible(false);

    QHBoxLayout *hlineLayout = new QHBoxLayout;
    hlineLayout->setSpacing(0);
    hlineLayout->setContentsMargins(50, 0, 50, 5);
    hlineLayout->addWidget(m_splitline);

    m_hhLayout->addLayout(hlineLayout);
    // 参考实现在此之后还有宜/忌两块（m_yiDownLayout / m_jiDownLayout），
    // gxde 的农历服务不提供宜忌数据，故未移植。
    m_hhLayout->addStretch();

    setLayout(m_hhLayout);
}

void CDayMonthView::initConnection()
{
    connect(m_prevButton, &QToolButton::clicked, this, &CDayMonthView::slotprev);
    connect(m_today, &QPushButton::clicked, this, &CDayMonthView::slottoday);
    connect(m_nextButton, &QToolButton::clicked, this, &CDayMonthView::slotnext);
    connect(m_dayMonthWidget, &CDayMonthWidget::signalChangeSelectDate,
            this, &CDayMonthView::signalChangeSelectDate);
}

/**
 * @brief CDayMonthView::updateDateShow     更新月/天界面显示
 */
void CDayMonthView::updateDateShow()
{
    QLocale locale;
    m_currentMouth->setTextStr(locale.monthName(m_selectDate.month(), QLocale::ShortFormat));
    m_currentDay->setTextStr(QString::number(m_selectDate.day()));

    if (m_selectDate.dayOfWeek() > 0)
        m_currentWeek->setTextStr(m_weeklist.at(m_selectDate.dayOfWeek() - 1));
    m_currentYear->setTextStr(m_selectDate.toString("yyyy/M"));

    // 按钮文案固定为 Today，不做「非今日时显示 Return Today」的切换
}

/**
 * @brief CDayMonthView::updateDateLunarDay     更新显示黄历信息
 */
void CDayMonthView::updateDateLunarDay()
{
    if (!m_huanglistate) {
        return;
    }
    if (m_lunarInfo.mGanZhiYear.isEmpty()
        && m_lunarInfo.mLunarMonthName.isEmpty()
        && m_lunarInfo.mLunarDayName.isEmpty()) {
        m_currentLuna->setTextStr(QString());
        return;
    }
    m_currentLuna->setTextStr(m_lunarInfo.mGanZhiYear + "年 " + "【" + m_lunarInfo.mZodiac + "年】"
                              + m_lunarInfo.mGanZhiMonth + "月 " + m_lunarInfo.mGanZhiDay + "日 ");
}

void CDayMonthView::changeSelectDate(const QDate &date)
{
    emit signalChangeSelectDate(date);
}

void CDayMonthView::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() < 0) {
        // 向下滚动 = 后一天
        changeSelectDate(m_selectDate.addDays(1));
    } else {
        // 向上滚动 = 前一天
        changeSelectDate(m_selectDate.addDays(-1));
    }
}

void CDayMonthView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    const int labelwidth = width();
    const int labelheight = height();
    const QPalette aniPa = palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setBrush(aniPa.window());
    painter.setPen(Qt::NoPen);
    QPainterPath painterPath;
    painterPath.moveTo(0, 0);
    painterPath.lineTo(0, labelheight);
    painterPath.lineTo(labelwidth - m_radius, labelheight);
    painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2,
                            m_radius * 2, m_radius * 2), 270, 90);
    painterPath.lineTo(labelwidth, m_radius);
    painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
    painterPath.lineTo(0, 0);
    painterPath.closeSubpath();
    painter.drawPath(painterPath);
    painter.restore();
}

void CDayMonthView::slotprev()
{
    changeSelectDate(m_selectDate.addMonths(-1));
}

void CDayMonthView::slotnext()
{
    changeSelectDate(m_selectDate.addMonths(1));
}

void CDayMonthView::slottoday()
{
    changeSelectDate(m_currentDate);
}

// ------------------------------------------------------------ CDayMonthWidget

CDayMonthWidget::CDayMonthWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout;
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            QWidget *cell = new QWidget(this);
            cell->installEventFilter(this);
            m_gridLayout->addWidget(cell, r, c, 1, 1);
            m_cellList.append(cell);
        }
    }
    this->setLayout(m_gridLayout);
    setFocusPolicy(Qt::StrongFocus);
    // 设置最大高度
    setMaximumHeight(250);
}

CDayMonthWidget::~CDayMonthWidget() = default;

void CDayMonthWidget::setTheMe(int type)
{
    m_currentDayTextColor = DDE25::systemActiveColor();
    if (type == 0 || type == 1) {
        m_defaultTextColor = Qt::black;
        m_selectedTextColor = Qt::white;
        m_notCurrentTextColor = "#b2b2b2";
        m_ceventColor = QColor(255, 93, 0);
    } else if (type == 2) {
        m_defaultTextColor = "#C0C6D4";
        m_selectedTextColor = "#B8D3FF";
        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_ceventColor = QColor(204, 77, 3);
    }
    update();
}

void CDayMonthWidget::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
{
    m_showDays = showDate;
    m_selectDate = selectDate;
    m_currentDate = currentDate;
    // 当前选择 index
    m_selectedCell = m_showDays.indexOf(m_selectDate);
    update();
}

void CDayMonthWidget::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
{
    m_vlineflag = hasScheduleFlag;
    update();
}

const QString CDayMonthWidget::getCellDayNum(int pos)
{
    return QString::number(m_showDays[pos].day());
}

const QDate CDayMonthWidget::getCellDate(int pos)
{
    return m_showDays[pos];
}

void CDayMonthWidget::paintCell(QWidget *cell)
{
    const QRect rect = cell->rect();
    const int pos = m_cellList.indexOf(cell);
    if (pos < 0 || pos >= m_showDays.size()) {
        return;
    }
    const bool isSelectedCell = pos == m_selectedCell;
    const bool isCurrentDay = getCellDate(pos) == m_currentDate;

    QPainter painter(cell);
    painter.setRenderHint(QPainter::Antialiasing);

    // 选中格的圆底
    if (isSelectedCell) {
        const qreal r = rect.width() > rect.height() ? rect.height() * 0.9 : rect.width() * 0.9;
        const qreal x = rect.x() + (rect.width() - r) / 2;
        const qreal y = rect.y() + (rect.height() - r) / 2;
        const QRectF fillRect = QRectF(x, y, r, r).marginsRemoved(QMarginsF(1.5, 2.5, 1.5, 1.5));
        painter.save();
        painter.setBrush(QBrush(DDE25::systemActiveColor()));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(fillRect);
        painter.restore();
        if (m_isFocus) {
            // 绘制焦点效果：在选中圆外再画一圈
            QPen pen;
            pen.setWidth(2);
            pen.setColor(DDE25::systemActiveColor());
            painter.setPen(pen);
            const QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2,
                                   fillRect.width() + 4, fillRect.height() + 4);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(focusRect);
        }
    }

    painter.setPen(Qt::SolidLine);

    const QString dayNum = getCellDayNum(pos);

    // 绘制日期数字
    if (isSelectedCell) {
        painter.setPen(m_selectedTextColor);
    } else if (isCurrentDay) {
        painter.setPen(m_currentDayTextColor);
    } else {
        if (m_selectDate.month() == getCellDate(pos).month())
            painter.setPen(m_defaultTextColor);
        else
            painter.setPen(m_notCurrentTextColor);
    }

    painter.setFont(m_dayNumFont);
    painter.drawText(rect, Qt::AlignCenter, dayNum);

    // 右上角日程小圆点
    if (m_vlineflag.size() == DDEDayCalendar::PainterCellNum && m_vlineflag[pos]) {
        if (m_selectDate.month() == getCellDate(pos).month()) {
            painter.save();
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_ceventColor);
            pen.setBrush(QBrush(m_ceventColor));
            painter.setPen(Qt::NoPen);
            int r = int(cell->width() * 0.1);
            if (r < 4) {
                r = 4;
            } else if (r > 7) {
                r = 7;
            }
            painter.drawEllipse(cell->width() - r - 6, 4, r, r);
            painter.restore();
        }
    }
}

bool CDayMonthWidget::eventFilter(QObject *o, QEvent *e)
{
    if (m_showDays.size() != DDEDayCalendar::PainterCellNum)
        return false;
    QWidget *cell = qobject_cast<QWidget *>(o);
    if (cell && m_cellList.contains(cell)) {
        const int pos = m_cellList.indexOf(cell);
        const QDate date = m_showDays[pos];
        // 需要显示的时间早于 1900 年则退出
        if (date.year() < DDECalendar::QueryEarliestYear)
            return false;
        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            // 参考实现这里会开始拖拽新建日程；本阶段无日程数据层，只记录按下的位置
            m_startPos = static_cast<QMouseEvent *>(e)->pos();
        } else if (e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            // 点击（未拖出格子）则跳转日期
            if (mouseEvent->button() == Qt::LeftButton && cell->rect().contains(mouseEvent->pos())) {
                cellClicked(cell);
            }
        }
    }
    return false;
}

void CDayMonthWidget::resizeEvent(QResizeEvent *event)
{
    // 获取每个时间 widget 的高度和宽度
    const qreal width = this->width() / 7.0;
    const qreal height = this->height() / 6.0;
    const qreal r = width > height ? height * 0.9 : width * 0.9;
    // 根据高度和宽度设置时间字体的大小
    m_dayNumFont.setPixelSize(qRound(12 + (r - 18) * 6 / 17.0));
    QWidget::resizeEvent(event);
}

void CDayMonthWidget::focusInEvent(QFocusEvent *event)
{
    switch (event->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
    case Qt::ActiveWindowFocusReason:
        m_isFocus = true;
        break;
    default:
        m_isFocus = false;
        break;
    }
    update();
}

void CDayMonthWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_isFocus = false;
    update();
}

void CDayMonthWidget::keyPressEvent(QKeyEvent *event)
{
    if (m_isFocus) {
        switch (event->key()) {
        case Qt::Key_Left:
            emit signalChangeSelectDate(m_selectDate.addDays(-1));
            break;
        case Qt::Key_Right:
            emit signalChangeSelectDate(m_selectDate.addDays(1));
            break;
        case Qt::Key_Up:
            emit signalChangeSelectDate(m_selectDate.addDays(-7));
            break;
        case Qt::Key_Down:
            emit signalChangeSelectDate(m_selectDate.addDays(7));
            break;
        default:
            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void CDayMonthWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    m_isFocus = false;
    if (event->button() & Qt::LeftButton)
        m_startPos = event->pos();
}

void CDayMonthWidget::cellClicked(QWidget *cell)
{
    const int pos = m_cellList.indexOf(cell);
    if (pos == -1) {
        return;
    }
    setSelectedCell(pos);
}

void CDayMonthWidget::setSelectedCell(int index)
{
    if (m_selectedCell == index) {
        return;
    }
    emit signalChangeSelectDate(m_showDays.at(index));
}
