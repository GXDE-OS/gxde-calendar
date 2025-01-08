#include "holidayapi.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>

HolidayAPI::HolidayAPI()
    : m_http(new QNetworkAccessManager(this))
{
    m_configPath = QDir::homePath() + "/.config/GXDE/gxde-calendar";
    initHolidayDataOffline();
    initSentenseOnline();
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
    emit refreshDataFinished();
}

// 请求一言数据
void HolidayAPI::initSentenseOnline()
{
    /*QUrl url("https://v1.hitokoto.cn/?c=i&c=k");
    QNetworkRequest request(url);
    QNetworkReply *reply = m_http->get(request);*/

    QUrl url("http://dict.youdao.com/infoline/style/cardList");
    QUrlQuery query;
    query.addQueryItem("apiversion", "3.0");
    query.addQueryItem("client", "deskdict");
    query.addQueryItem("style", "daily");
    query.addQueryItem("lastId", "0");
    query.addQueryItem("keyfrom", "deskdict.8.1.2.0");
    query.addQueryItem("size", "1");
    url.setQuery(query.toString(QUrl::FullyEncoded));

    QNetworkRequest request(url);
    QNetworkReply *reply = m_http->get(request);

    connect(reply, &QNetworkReply::finished, this, &HolidayAPI::handleQuerySentenseFinished);
}

// 用于请求假期数据
void HolidayAPI::initHolidayDataOnline()
{
    QUrl url("https://datetime.gxde.top/holiday.json");
    QNetworkRequest request(url);
    QNetworkReply *reply = m_http->get(request);

    connect(reply, &QNetworkReply::finished, this, &HolidayAPI::handleQueryHolidayFinished);
}

QStringList HolidayAPI::getDailySentense()
{
    return QStringList() << m_sentense << m_sentenseWho << m_sentenseEn << m_sentenseWho;
}

// 处理返回的 json 一言数据
void HolidayAPI::handleQuerySentenseFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error() != QNetworkReply::NoError) {
        // 输出报错
        qDebug() << reply->errorString();
        return;
    }

    /*QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    if (document.isEmpty()) {
        return;
    }
    m_sentense = document.object().value("hitokoto").toString();
    m_sentenseWho = document.object().value("from_who").toString();*/

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    QJsonObject object = document.array().at(0).toObject();

    const QString title = object.value("title").toString();
    const QString summary = object.value("summary").toString();

    m_sentense = summary;
    m_sentenseEn = title;

    // 发送信号提示以告诉其它部件资源已加载完成
    emit refreshDataFinished();
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
