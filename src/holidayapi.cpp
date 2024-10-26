#include "holidayapi.h"
#include <QDir>
#include <QFile>

HolidayAPI::HolidayAPI()
    : m_http(new QNetworkAccessManager(this))
{
    m_configPath = QDir::homePath() + "/.config/GXDE/gxde-calendar";
    initHolidayDataOffline();
    initHolidayDataOnline();
}

HolidayAPI::~HolidayAPI()
{
    delete m_http;
}

// 用于读取缓存至本地的假期数据
void HolidayAPI::initHolidayDataOffline()
{
    if (!QFile::exists(m_configPath + "/holiday.json")) {
        // 如果缓存文件不存在则不读取
        return;
    }
    // 读取缓存文件
    QFile file(m_configPath + "/holiday.json");
    file.open(QFile::ReadOnly);
    // 读取并缓存 json
    QJsonObject jsonData = QJsonDocument::fromJson(file.readAll()).object();
    if (!networkReaded) {
        // 必须保证在网络加载成功前读取，否则会一直使用本地过时的数据
        m_holidayData = jsonData;
    }
    file.close();
}

// 用于请求假期数据
void HolidayAPI::initHolidayDataOnline()
{
    QUrl url("https://datetime.gxde.org/holiday.json");
    QNetworkRequest request(url);
    QNetworkReply *reply = m_http->get(request);

    connect(reply, &QNetworkReply::finished, this, &HolidayAPI::handleQueryHolidayFinished);
}

// 处理返回的 json 假期数据
void HolidayAPI::handleQueryHolidayFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error() != QNetworkReply::NoError) {
        // 输出报错
        qDebug() << reply->errorString();
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    if (document.isEmpty()) {
        return;
    }
    // 设置锁以不允许读取本地缓存数据
    networkReaded = true;
    setData(document);
    // 发送信号提示以告诉其它部件资源已加载完成
    emit refreshDataFinished();
}

bool HolidayAPI::isWeekend(QDate date)
{
    QLocale locale = QLocale::English;
    QString ddd = locale.toString(date, "ddd");  // 获取当前星期英语缩写
    if (ddd == "Sun" || ddd == "Sat") {
        return true;
    }
    return false;
}

HolidayAPI::DayStatus HolidayAPI::getDayStatus(QDate date)
{
    // 获取日期
    QString year = date.toString("yyyy");
    QString month = date.toString("MM");
    QString day = date.toString("dd");
    // json 例子
    /*{
    "2024": {
        "10": {
            "1": "H"
        }
    }
    }*/
    // 处理数据
    QJsonValue yearObject = m_holidayData.value(year);
    if (yearObject.isUndefined()) {
        return isWeekend(date) ? DayStatus::normalHoliday : DayStatus::normalWork;
    }
    QJsonValue monthObject = yearObject.toObject().value(month);
    if (monthObject.isUndefined()) {
        return isWeekend(date) ? DayStatus::normalHoliday : DayStatus::normalWork;
    }
    QJsonValue dayObject = monthObject.toObject().value(day);
    if (dayObject.isUndefined()) {
        return isWeekend(date) ? DayStatus::normalHoliday : DayStatus::normalWork;
    }
    if (dayObject.toString() == "H") {
        return DayStatus::holiday;
    }
    return DayStatus::work;
}

// 用于设置和缓存当前数据的
void HolidayAPI::setData(QJsonDocument data)
{
    m_holidayData = data.object();
    // 存储至本地
    // 如果文件夹不存在则创建
    if (!QDir().exists(m_configPath)) {
        QDir().mkpath(m_configPath);
    }
    // 写入文件
    QFile file(m_configPath + "/holiday.json");
    file.open(QFile::WriteOnly);
    file.write(data.toJson());
    file.close();
}

bool HolidayAPI::isWork(QDate date)
{
    DayStatus info = getDayStatus(date);
    return (info == DayStatus::normalWork || info == DayStatus::work);
}

bool HolidayAPI::isHoliday(QDate date)
{
    DayStatus info = getDayStatus(date);
    return (info == DayStatus::normalHoliday || info == DayStatus::holiday);
    return true;
}