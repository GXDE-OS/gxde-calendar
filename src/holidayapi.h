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


signals:
    void refreshDataFinished();

private:
    void initHolidayDataOffline();
    void initHolidayDataOnline();
    void handleQueryHolidayFinished();
    void setData(QJsonDocument data);

    bool networkReaded = false;

    QNetworkAccessManager *m_http;
    QJsonObject m_holidayData;

    QString m_configPath;
};

#endif // HOLIDAYAPI_H
