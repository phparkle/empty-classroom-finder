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

void HttpClient::downloadAll(const QUrl indexUrl) {
    requestIndex(indexUrl);
}

void HttpClient::requestIndex(const QUrl indexUrl) {
    qDebug() << "REQUEST" << indexUrl;
    QNetworkReply* const reply = manager.get(QNetworkRequest(indexUrl));
    connect(reply, &QNetworkReply::finished,
            this, &HttpClient::handleIndexReplyFinished);
}

void HttpClient::requestDept(const QUrl deptUrl) {
    qDebug() << "REQUEST" << deptUrl;
    QNetworkReply* const reply = manager.get(QNetworkRequest(deptUrl));
    connect(reply, &QNetworkReply::finished,
            this, &HttpClient::handleDeptReplyFinished);
}

void HttpClient::handleIndexReplyFinished() {
    QNetworkReply* const reply = static_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "REPLY" << reply->url();
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
}

void HttpClient::handleDeptReplyFinished() {
    QNetworkReply* const reply = static_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "REPLY" << reply->url();
    const QDir htmlDir = getHtmlDir();
    if (!htmlDir.exists()) htmlDir.mkpath(".");
    QString filename = reply->url().path().section('/', -1).append(".html");
    QFile file(htmlDir.filePath(filename));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    QMutexLocker locker(&mutex);
    if (--deptsRemaining == 0)
        emit finished();
}
