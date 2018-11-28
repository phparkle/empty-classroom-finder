#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>

class HttpClient : public QObject
{
    Q_OBJECT

public:
    HttpClient(QObject* parent = nullptr);

    void downloadAll(const QUrl indexUrl);

signals:
    void finished();

private slots:
    void handleIndexReplyFinished();
    void handleDeptReplyFinished();

private:
    void requestIndex(const QUrl indexUrl);
    void requestDept(const QUrl deptUrl);

    QNetworkAccessManager manager;
    int deptsRemaining = 0;
    QMutex mutex;
};

#endif // HTTP_CLIENT
