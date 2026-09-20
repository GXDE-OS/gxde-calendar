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

#ifndef SRC_VIEWSWITCHER_H_
#define SRC_VIEWSWITCHER_H_

#include <QWidget>

class QButtonGroup;
class QPushButton;

class ViewSwitcher : public QWidget {
    Q_OBJECT
public:
    explicit ViewSwitcher(QWidget *parent = nullptr);

    void setLabels(const QStringList &labels);
    void setCurrentIndex(int index);
    int currentIndex() const;

signals:
    void currentChanged(int index);

private:
    QButtonGroup *m_group = nullptr;
    QList<QPushButton *> m_buttons;
};

#endif  // SRC_VIEWSWITCHER_H_
