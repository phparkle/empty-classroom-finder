// Filename: httpclient.h
//
// The HTTPClient class downloads HTML files from the class schedule website.

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

    // Downloads all HTML files from the class schedule website.
    void downloadAll();

signals:
    // Emitted to signify download progress
    void statusChanged(QString message);

    // Emitted when all downloads finish
    void finished();

private slots:
    // Parses the index file and initiates requests to class schedules for each
    // department
    void handleIndexReplyFinished();

    // Saves the department class schedule to disk
    void handleDeptReplyFinished();

private:
    // Starts HTTP requests
    void requestIndex(const QUrl indexUrl);
    void requestDept(const QUrl deptUrl);

    // Returns the URL of the index page for the current semester
    static QString getIndexUrl();

    // Manages network connections
    QNetworkAccessManager manager;

    // The number of department schedules remaining to be downloaded
    int deptsRemaining = 0;
};

#endif // HTTP_CLIENT
