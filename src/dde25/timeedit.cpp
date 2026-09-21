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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/timeedit.*）。
 * 差异：initUI() 末尾给 m_timeEdit 补一个隐藏的占位 QLineEdit，避开 Qt6 在
 *       QComboBox::setLineEdit() 抢走 QAbstractSpinBox 的编辑框之后的空指针解引用
 *       （原因见 initUI() 中的注释）。
 */

#include "timeedit.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QFocusEvent>
#include <QFrame>
#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>

//视图容器最大高度
const int viewContainerMaxHeight = 305;

CTimeEdit::CTimeEdit(QWidget *parent)
    : QComboBox(parent)
    , m_timeFormat("hh:mm")
    , m_timeEdit(new CCustomTimeEdit())
    , m_hasFocus(false)
    , m_miniTime(QTime(0, 0, 0))
{
    m_timeEdit->setObjectName("TimeEdit");
    m_timeEdit->setAccessibleName("TimeEdit");
    initUI();
    initConnection();
}

CTimeEdit::~CTimeEdit()
{
    delete m_timeEdit;
}

void CTimeEdit::setMineTime(const QTime &mineTime)
{
    m_miniTime = mineTime;
}

void CTimeEdit::setTime(const QTime &time)
{
    m_time = time;
    setSelectItem(m_time);
    m_timeEdit->getLineEdit()->setText(time.toString(m_timeFormat));
}

QTime CTimeEdit::getTime()
{
    //获取显示的text
    QString timetext = m_timeEdit->getLineEdit()->displayText();
    //将text转换为时间
    m_time = QTime::fromString(timetext, m_timeFormat);
    return m_time;
}

void CTimeEdit::updateListItem(bool isShowTimeInterval)
{
    m_isShowTimeInterval = isShowTimeInterval;
    QTime topTimer(m_miniTime);
    if (!m_isShowTimeInterval) {
        // 根据开始时间的分钟数设置结束时间下拉列表的分钟数，当前默认为0,故先注释
        // int m = m_miniTime.minute() >= 30 ? m_miniTime.minute() - 30 : m_miniTime.minute();
        int m = 0;
        topTimer = QTime(0, m);
    }
    //清除列表
    clear();
    for (int i = 0; i < 48; ++i) {
        qreal timeInterval = i * 0.5;
        QString timeIntervalStr;
        if (m_isShowTimeInterval) {
            if (timeInterval < 1) {
                timeIntervalStr = tr("(%1 mins)").arg(i * 30);
            } else if (qFuzzyCompare(timeInterval, 1)) {
                timeIntervalStr = tr("(%1 hour)").arg(timeInterval);
            } else {
                timeIntervalStr = tr("(%1 hours)").arg(timeInterval);
            }
        } else {
            timeIntervalStr = "";
        }
        QString userData = topTimer.addSecs(i * 30 * 60).toString(m_timeFormat);
        QString showStr = userData + timeIntervalStr;
        addItem(showStr, userData);
    }
}

void CTimeEdit::setTimeFormat(int value)
{
    //获取edit的当前时间
    QTime editCurrentTime = getTime();
    //根据value值,设置时间显示格式
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
    //先更新列表信息，更新列表信息后再设置编辑框显示时间
    updateListItem(m_isShowTimeInterval);
    m_timeEdit->setDisplayFormat(m_timeFormat);
    //设置改变格式后的时间
    setTime(editCurrentTime);
}

void CTimeEdit::slotFocusDraw(bool showFocus)
{
    m_hasFocus = showFocus;
    update();
}

void CTimeEdit::setSelectItem(const QTime &time)
{
    //若有则设置选中，若没有则取消选中设置定位到相近值附近
    int similarNumber = -1;
    int diff = 24 * 60 * 60 * 1000;
    for (int i = 0 ; i < count(); ++i) {
        QVariant &&userData = itemData(i);
        QTime &&listTime = QTime::fromString(userData.toString(), m_timeFormat);
        int &&timeDiff = qAbs(listTime.msecsTo(time));
        //获取时间差较小的值
        //如果时间小于第一项的时间则不算统计，
        //比如 第一项时间位01:00 比对时间为00:59，时间差应该为23：59 而不是00：01，所以应该定位到最后一项
        if (timeDiff < diff && (!(time < listTime && 0 == i))) {
            diff = timeDiff;
            similarNumber = i;
        }
    }
    if (diff == 0) {
        setCurrentIndex(similarNumber);
    } else {
        setCurrentIndex(-1);
    }
    scrollPosition = this->model()->index(similarNumber, 0);
}

void CTimeEdit::slotSetPlainText(const QString &arg)
{
    QString userData = currentData().toString();
    if (userData.isEmpty()) {
        this->lineEdit()->setText(arg);
    } else {
        this->lineEdit()->setText(userData);
    }
}

void CTimeEdit::slotActivated(const QString &arg)
{
    slotSetPlainText(arg);
    emit signaleditingFinished();
}

void CTimeEdit::slotEditingFinished()
{
    setTime(m_timeEdit->time());
    emit signaleditingFinished();
}

void CTimeEdit::initUI()
{
    //关闭自动补全
    this->setCompleter(nullptr);
    //设置edit的宽度
    m_timeEdit->getLineEdit()->setFixedWidth(100);
    m_timeEdit->setDisplayFormat(m_timeFormat);
    updateListItem(m_isShowTimeInterval);
    setLineEdit(m_timeEdit->getLineEdit());
    m_timeEdit->setParent(this);
    setInsertPolicy(QComboBox::NoInsert);

    // Qt6 适配：上面 setLineEdit() 把 m_timeEdit 自己的 QLineEdit 抢过来当编辑框，
    // 它的父对象随之变成 this，于是 m_timeEdit 名下再没有 QLineEdit 子对象了。
    // 而 QStyleSheetStyle::embeddedWidget() 对 QAbstractSpinBox 的实现是
    //     return sb->findChild<QLineEdit *>();          // 此时返回 nullptr
    // Qt 6.8 的 QStyleSheetStyle::setPalette() 又只判 `ew != w` 就把它继续传下去：
    //     QWidget *ew = embeddedWidget(w);
    //     updateStyleSheetFont(w);
    //     if (ew != w) updateStyleSheetFont(ew);        // qstylesheetstyle.cpp:2745
    // 而 updateStyleSheetFont() 第一句就是 `w->objectName()`，nullptr 直接段错误。
    // 触发点是 QWidget::ensurePolished()——它无条件 polish 自己与全部子对象，
    // 所以只要这个 spinbox 挂在树里，任何一次 setVisible()/show() 都会踩到。
    // 参考实现基于 Qt5，Qt5 的 setPalette() 里没有 updateStyleSheetFont(ew) 这次调用，
    // 所以同样的写法在 Qt5 上不炸。
    // 这里补一个隐藏的占位 QLineEdit 挂在 m_timeEdit 名下，只为让 findChild 找得到：
    // 不参与显示，也不去动 QAbstractSpinBox::lineEdit()（即真正的编辑框仍归 combo）。
    auto *placeholderLineEdit = new QLineEdit(m_timeEdit);
    placeholderLineEdit->hide();
}

void CTimeEdit::initConnection()
{
    connect(m_timeEdit, &CCustomTimeEdit::signalUpdateFocus, this, &CTimeEdit::slotFocusDraw);
    connect(m_timeEdit->getLineEdit(), &QLineEdit::editingFinished, this,
            &CTimeEdit::slotEditingFinished);
    connect(this, &CTimeEdit::textActivated, this, &CTimeEdit::slotActivated);
}

void CTimeEdit::showPopup()
{
    QComboBox::showPopup();
    //获取下拉视图容器
    QFrame *viewContainer = this->findChild<QFrame *>();
    if (viewContainer) {
        //移动前先隐藏
        viewContainer->hide();
        //如果显示视图容器则设置高度
        viewContainer->setFixedHeight(viewContainerMaxHeight);
        //设置最大高度
        viewContainer->setMaximumHeight(viewContainerMaxHeight + 1);
        //获取combobox底部坐标
        QPoint showPoint = mapToGlobal(this->rect().bottomLeft());
        //控制视图容器宽度 ，根据字体大小调整宽度
        int maxLen = 0;
        QFontMetrics fontMet(view()->font());
        for (int i = 0 ; i < count() ; ++i) {
            int &&itemWidth = fontMet.horizontalAdvance(this->itemText(i));
            maxLen = qMax(maxLen, itemWidth);
        }
        maxLen += 45;   //选项前√占用的大小
        //如果宽度小于box宽度则设置位box宽度
        maxLen = qMax(maxLen, this->width());
        viewContainer->setFixedWidth(maxLen);
        //将视图容器移动到combobox的底部
        viewContainer->move(showPoint.x(), showPoint.y());
        //显示
        viewContainer->show();
    }
    //因改变了容器的高度，所以需要重新定位当前位置
    if (this->view()->currentIndex() == scrollPosition) {
        this->view()->scrollTo(scrollPosition, QAbstractItemView::PositionAtCenter);
    } else {
        this->view()->scrollTo(scrollPosition, QAbstractItemView::PositionAtTop);
    }
}

void CTimeEdit::focusInEvent(QFocusEvent *event)
{
    QComboBox::focusInEvent(event);
    //    如果为tab焦点进入则选中时间
    if (event->reason() == Qt::TabFocusReason) {
        lineEdit()->setFocus(Qt::TabFocusReason);
    }
}

void CTimeEdit::paintEvent(QPaintEvent *e)
{
    QComboBox::paintEvent(e);
    //如果有焦点则设置焦点显示效果
    if (m_hasFocus) {
        QPainter painter(this);
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void CTimeEdit::resizeEvent(QResizeEvent *e)
{
    QComboBox::resizeEvent(e);
    m_timeEdit->setFixedHeight(this->height());
}
