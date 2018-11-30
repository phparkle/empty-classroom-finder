#include "global.h"
#include "httpclient.h"

#include <htmlcxx/html/ParserDom.h>
#include <hcxselect/hcxselect.h>

#include <QMutexLocker>
#include <QNetworkReply>

#include <string>

using namespace htmlcxx;

HttpClient::HttpClient(QObject* parent) : QObject(parent)
{
}

QString HttpClient::getIndexUrl() {
    const QDate today = QDate::currentDate();
    const QString year = today.toString("yy");
    const QString semester = today.month() > 6 ? "10" : "30";
    return QString("https://w5.ab.ust.hk/wcq/cgi-bin/%1%2/").arg(year).arg(semester);
}

void HttpClient::downloadAll() {
    if (getHtmlDir().exists())
        emit finished();
    else requestIndex(getIndexUrl());
}

void HttpClient::requestIndex(const QUrl indexUrl) {
    emit statusChanged(QString("Downloading %1").arg(indexUrl.toString()));
    QNetworkReply* const reply = manager.get(QNetworkRequest(indexUrl));
    connect(reply, &QNetworkReply::finished,
            this, &HttpClient::handleIndexReplyFinished);
}

void HttpClient::requestDept(const QUrl deptUrl) {
    QNetworkReply* const reply = manager.get(QNetworkRequest(deptUrl));
    connect(reply, &QNetworkReply::finished,
            this, &HttpClient::handleDeptReplyFinished);
}

void HttpClient::handleIndexReplyFinished() {
    QNetworkReply* const reply = static_cast<QNetworkReply*>(QObject::sender());
    const std::string html = reply->readAll().toStdString();
    HTML::ParserDom parser;
    const tree<HTML::Node>& dom = parser.parseTree(html);
    const std::string query = "#navigator .depts a";
    const hcxselect::NodeSet nodes = hcxselect::select(dom, query);
    getHtmlDir().removeRecursively();
    deptsRemaining = static_cast<int>(nodes.size());
    for (hcxselect::Node* const node : nodes) {
        HTML::Node& a = node->data;
        a.parseAttributes();
        const std::string deptHref = a.attribute("href").second;
        const QUrl deptUrl = QString::fromStdString(deptHref);
        requestDept(reply->url().resolved(deptUrl));
    }
    reply->deleteLater();
}

void HttpClient::handleDeptReplyFinished() {
    QNetworkReply* const reply = static_cast<QNetworkReply*>(QObject::sender());
    const QUrl deptUrl = reply->url();
    emit statusChanged(QString("Downloading %1").arg(deptUrl.toString()));
    const QDir htmlDir = getHtmlDir();
    if (!htmlDir.exists()) htmlDir.mkpath(".");
    QString filename = deptUrl.path().section('/', -1).append(".html");
    QFile file(htmlDir.filePath(filename));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    if (--deptsRemaining == 0)
        emit finished();
    reply->deleteLater();
}
