/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 * 本文件为 GXDE 日历原创。
 */

#include "sidebarschedulelist.h"

#include "dde25common.h"
#include "schedulecolors.h"
#include "schedulequery.h"
#include "uitheme.h"

#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include <algorithm>

namespace {

// 行间距；行高与行内文字行高取 DDE25::Metric，不在这里另立一套
const int ItemSpacing = 2;
// 侧栏左右留白，和迷你月历的观感对齐
const int ListLeftRightMargin = 10;
// 左侧类型色条的宽度（月视图的日程块也是同宽竖条）
const int ColorBarWidth = DDE25::Metric::ColorBar;

} // namespace

SidebarScheduleItem::SidebarScheduleItem(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(DDE25::Metric::ScheduleRowHeight);
    setCursor(Qt::PointingHandCursor);
    // 行上没有子控件，鼠标移进移出要靠 enterEvent/leaveEvent 自己重绘
    setAttribute(Qt::WA_Hover, true);
}

void SidebarScheduleItem::setData(const DSchedule::Ptr &schedule)
{
    m_schedule = schedule;

    // 颜色优先取日程自带的 accountColor（数据层目前不填），否则按类型查色板；
    // 与 CMonthScheduleItem/CScheduleItem 取色方式一致
    QColor color = schedule.isNull() ? QColor() : QColor(schedule->accountColor());
    if (!color.isValid() && !schedule.isNull()) {
        color = DDE25::scheduleColorByType(schedule->scheduleTypeID()).orginalColor;
    }
    m_color = color.isValid() ? color : DDE25::accentColor();

    update();
}

QString SidebarScheduleItem::timeText() const
{
    if (m_schedule.isNull()) {
        return QString();
    }

    if (m_schedule->allDay()) {
        return tr("All Day");
    }

    // 本项目统一用 24 小时制（同 CScheduleItem，参考实现那边还有个 12 小时制设置项）
    const QDateTime start = m_schedule->dtStart().toLocalTime();
    QString text = start.time().toString("hh:mm");

    const QDateTime end = m_schedule->dtEnd().toLocalTime();
    // 跨天日程只写开始时间，写了结束时间反而看不出是哪天的
    if (end.date() == start.date()) {
        text += QStringLiteral(" - %1").arg(end.time().toString("hh:mm"));
    }

    return text;
}

void SidebarScheduleItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_hover) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(DDE25::hoverColor());
        painter.drawRoundedRect(rect(), DDE25::Metric::RadiusItem,
                                DDE25::Metric::RadiusItem);
    }

    // 左侧类型色条：与月视图日程块一样画在行首
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_color);
    painter.drawRoundedRect(QRectF(0, 8, ColorBarWidth, height() - 16), 2, 2);

    const QColor titleColor = DDE25::textColor();
    const QColor timeColor = DDE25::textColor(DDE25::TextSecondary);

    const int textLeft = ColorBarWidth + 8;
    const int textWidth = width() - textLeft - 6;
    // 两行文字共用同一个行高，上下留白对称：整行只有这一个行高值
    const int lineH = DDE25::Metric::ScheduleRowLine;
    const int topPad = (DDE25::Metric::ScheduleRowHeight - lineH * 2) / 2;

    QFont font = painter.font();

    font.setPixelSize(13);
    painter.setFont(font);
    painter.setPen(titleColor);
    const QString title = m_schedule.isNull() ? QString() : m_schedule->summary();
    painter.drawText(QRect(textLeft, topPad, textWidth, lineH),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(title, Qt::ElideRight, textWidth));

    font.setPixelSize(11);
    painter.setFont(font);
    painter.setPen(timeColor);
    painter.drawText(QRect(textLeft, topPad + lineH, textWidth, lineH),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(timeText(), Qt::ElideRight, textWidth));
}

void SidebarScheduleItem::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_schedule.isNull()) {
        return;
    }

    QMenu menu(this);
    QAction *edit = menu.addAction(tr("Edit"));
    QAction *remove = menu.addAction(tr("Delete"));
    QAction *chosen = menu.exec(event->globalPos());
    if (chosen == nullptr) {
        return;
    }

    if (chosen == remove) {
        emit signalDeleteClicked(m_schedule);
        return;
    }
    if (chosen == edit) {
        emit signalClicked(m_schedule);
    }
}

void SidebarScheduleItem::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_hover = true;
    update();
}

void SidebarScheduleItem::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_hover = false;
    update();
}

void SidebarScheduleItem::mouseReleaseEvent(QMouseEvent *event)
{
    // 按下后拖到行外再松手不算点击
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint())) {
        emit signalClicked(m_schedule);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

SidebarScheduleList::SidebarScheduleList(QWidget *parent)
    : QWidget(parent)
{
    m_dateLabel = new QLabel(this);
    QFont font = m_dateLabel->font();
    font.setPixelSize(12);
    m_dateLabel->setFont(font);
    m_dateLabel->setStyleSheet(
        QStringLiteral("QLabel { color: %1; }").arg(DDE25::css(DDE25::textColor())));

    m_addButton = new QPushButton(this);
    m_addButton->setIcon(QIcon(DDE25::themeType() == 2
                                   ? ":/resources/icon/dde_calendar_create_dark.svg"
                                   : ":/resources/icon/dde_calendar_create_light.svg"));
    m_addButton->setIconSize(QSize(DDE25::Metric::Icon, DDE25::Metric::Icon));
    m_addButton->setFixedSize(DDE25::Metric::ToolButton, DDE25::Metric::ToolButton);
    m_addButton->setToolTip(tr("New Schedule"));
    m_addButton->setFocusPolicy(Qt::NoFocus);
    m_addButton->setCursor(Qt::PointingHandCursor);
    // 与标题栏两个图标按钮同一份样式（见 uitheme.h）
    m_addButton->setStyleSheet(DDE25::toolButtonStyleSheet());
    connect(m_addButton, &QPushButton::clicked, this, [this] {
        if (m_date.isValid()) {
            emit signalCreateSchedule(m_date);
        }
    });

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(m_dateLabel, 0, Qt::AlignVCenter);
    headerLayout->addStretch();
    headerLayout->addWidget(m_addButton, 0, Qt::AlignVCenter);

    m_emptyLabel = new QLabel(tr("No Schedule"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(
        QStringLiteral("QLabel { color: %1; }")
            .arg(DDE25::css(DDE25::textColor(DDE25::TextTertiary))));

    m_itemContainer = new QWidget;
    m_itemContainer->setAutoFillBackground(false);
    m_itemLayout = new QVBoxLayout(m_itemContainer);
    // 右边留出滚动条的位置，免得日程条宽度在有/无滚动条时来回跳
    m_itemLayout->setContentsMargins(0, 0, 6, 8);
    m_itemLayout->setSpacing(ItemSpacing);
    m_itemLayout->addWidget(m_emptyLabel, 0, Qt::AlignHCenter);
    m_itemLayout->addStretch();

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_scrollArea->viewport()->setAutoFillBackground(false);
    m_scrollArea->setWidget(m_itemContainer);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ListLeftRightMargin, 0, ListLeftRightMargin, 0);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(6);
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_scrollArea, 1);
}

void SidebarScheduleList::setDate(const QDate &date)
{
    if (!date.isValid() || date == m_date) {
        return;
    }

    m_date = date;
    updateHeader();
    rebuild();
}

void SidebarScheduleList::refresh()
{
    if (!m_date.isValid()) {
        return;
    }

    updateHeader();
    rebuild();
}

void SidebarScheduleList::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 侧栏折叠期间日程可能已经变过，展开时补一次
    refresh();
}

void SidebarScheduleList::updateHeader()
{
    if (!m_date.isValid()) {
        m_dateLabel->clear();
        return;
    }

    // 与日视图的年份标签同一套写法：日期 + 星期
    m_dateLabel->setText(QLocale().toString(m_date, "yyyy/M/d dddd"));
}

void SidebarScheduleList::rebuild()
{
    for (SidebarScheduleItem *item : m_items) {
        m_itemLayout->removeWidget(item);
        item->deleteLater();
    }
    m_items.clear();

    const QMap<QDate, DSchedule::List> daySchedules = DDE25::querySchedules(m_date, m_date);
    DSchedule::List schedules = daySchedules.value(m_date);

    // 数据层的查询带了 order by，但排序字段是绑定参数（SQLite 当常量处理），
    // 出来的顺序不保证，这里自己排：全天在前，其余按开始时间，最后按标题
    std::sort(schedules.begin(), schedules.end(),
              [](const DSchedule::Ptr &left, const DSchedule::Ptr &right) {
                  if (left.isNull() || right.isNull()) {
                      return !left.isNull();
                  }
                  if (left->allDay() != right->allDay()) {
                      return left->allDay();
                  }
                  if (left->dtStart() != right->dtStart()) {
                      return left->dtStart() < right->dtStart();
                  }
                  return left->summary() < right->summary();
              });

    m_emptyLabel->setVisible(schedules.isEmpty());

    // 插在末尾的 stretch 之前
    const int insertAt = m_itemLayout->count() - 1;
    int offset = 0;
    for (const DSchedule::Ptr &schedule : schedules) {
        SidebarScheduleItem *item = new SidebarScheduleItem(m_itemContainer);
        item->setData(schedule);
        connect(item, &SidebarScheduleItem::signalClicked,
                this, &SidebarScheduleList::signalEditSchedule);
        connect(item, &SidebarScheduleItem::signalDeleteClicked,
                this, &SidebarScheduleList::signalDeleteSchedule);
        m_itemLayout->insertWidget(insertAt + offset, item);
        m_items.append(item);
        ++offset;
    }
}
