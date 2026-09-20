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
 */

#include "viewswitcher.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

ViewSwitcher::ViewSwitcher(QWidget *parent)
    : QWidget(parent) {
    setObjectName("ViewSwitcher");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "QWidget#ViewSwitcher {"
        "  background-color: white;"
        "  border: 1px solid rgba(0, 0, 0, 0.1);"
        "  border-radius: 4px;"
        "}"
        "QWidget#ViewSwitcher QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  color: #000000;"
        "  padding: 0px 6px;"
        "  border-radius: 3px;"
        "}"
        "QWidget#ViewSwitcher QPushButton:hover {"
        "  background-color: rgba(0, 0, 0, 0.05);"
        "}"
        "QWidget#ViewSwitcher QPushButton:checked {"
        "  background-color: #2ca7f8;"
        "  color: white;"
        "}");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    connect(m_group, &QButtonGroup::idClicked, this,
        &ViewSwitcher::currentChanged);
}

void ViewSwitcher::setLabels(const QStringList &labels) {
    for (QPushButton *button : m_buttons) {
        m_group->removeButton(button);
        delete button;
    }
    m_buttons.clear();

    QHBoxLayout *layout = qobject_cast<QHBoxLayout *>(this->layout());
    int id = 0;
    for (const QString &label : labels) {
        QPushButton *button = new QPushButton(label, this);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);

        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(button, 1);
        m_group->addButton(button, id++);
        m_buttons.append(button);
    }

    if (!m_buttons.isEmpty()) {
        m_buttons.first()->setChecked(true);
    }
}

void ViewSwitcher::setCurrentIndex(int index) {
    if (index < 0 || index >= m_buttons.size() || index == currentIndex()) {
        return;
    }

    m_buttons.at(index)->setChecked(true);
    emit currentChanged(index);
}

int ViewSwitcher::currentIndex() const {
    return m_group->checkedId();
}
