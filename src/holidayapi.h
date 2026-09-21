/*
 * Copyright (C) 2024 - 2025 gfdgd_xi <3025613752@qq.com>
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

#ifndef HOLIDAYAPI_H
#define HOLIDAYAPI_H

#include <QObject>
#include <QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

class HolidayAPI : public QObject
{
    Q_OBJECT

public:
    enum DayStatus {
        holiday,
        work,
        normal,
        normalWork,
        normalHoliday
    };

    HolidayAPI();
    ~HolidayAPI();

    bool isHoliday(QDate date);
    bool isWork(QDate date);
    bool isWeekend(QDate date);
    DayStatus getDayStatus(QDate date);

    QStringList getDailySentense();


signals:
    void refreshDataFinished();

private:
    void initHolidayDataOffline();
    void initHolidayDataOnline();
    void initSentenseOnline();
    void handleQueryHolidayFinished();
    void handleQuerySentenseFinished();
    void setData(QJsonDocument data);

    QString m_sentense;
    QString m_sentenseEn;
    QString m_sentenseWho;

    bool networkReaded = false;

    QNetworkAccessManager *m_http;
    QJsonObject m_holidayData;

    QString m_configPath;
};

#endif // HOLIDAYAPI_H
