#include "global.h"
#include "httpclient.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <hcxselect/hcxselect.h>
#include <htmlcxx/html/ParserDom.h>

#include <QDate>
#include <QDirIterator>
#include <QFile>
#include <QTime>

#include <string>

using namespace htmlcxx;

namespace {

struct WeekTime {
    int weekday;
    QTime time;
};

QList<QTime> parseTimes(const QString& dateTimeText) {
    QList<QTime> times;
    const int colon1 = dateTimeText.indexOf(':');
    const int colon2 = dateTimeText.indexOf(':', colon1 + 1);
    const QString timeText1 = dateTimeText.mid(colon1 - 2, 7);
    const QString timeText2 = dateTimeText.mid(colon2 - 2, 7);
    static const QString format = "hh:mmAP";
    const QTime t1 = QTime::fromString(timeText1, format);
    const QTime t2 = QTime::fromString(timeText2, format);
    for (QTime t = t1; t < t2; t = t.addSecs(60 * 30))
        times.append(t);
    return times;
}

QList<int> parseWeekdays(const QString& weekTimeText) {
    QList<int> daysOfWeek;
    static const QStringList labels = { "Mo", "Tu", "We", "Th", "Fr" };
    for (int i = 0; i < 5; ++i)
        if (weekTimeText.contains(labels[i]))
            daysOfWeek.append(i);
    return daysOfWeek;
}

bool isTodayInRange(QString dateText) {
    const QString dateText1 = dateText.left(11);
    const QString dateText2 = dateText.right(11);
    static const QString format = "dd-MMM-YYYY";
    const QDate d1 = QDate::fromString(dateText1, format);
    const QDate d2 = QDate::fromString(dateText2, format);
    const QDate today = QDate::currentDate();
    return today >= d1 && today <= d2;
}

QList<WeekTime> parseWeekTimes(hcxselect::Node* node) {
    QList<WeekTime> weekTimes;
    hcxselect::Node* weekTimeNode = node->first_child;
    if (weekTimeNode->next_sibling) {
        const std::string text = node->first_child->data.text();
        const QString dateText = QString::fromStdString(text);
        if (!isTodayInRange(dateText))
            return weekTimes;
        weekTimeNode = weekTimeNode->next_sibling->next_sibling;
    }
    const std::string text = weekTimeNode->data.text();
    const QString weekTimeText = QString::fromStdString(text);
    const QList<int> weekdays = parseWeekdays(weekTimeText);
    const QList<QTime> times = parseTimes(weekTimeText);
    for (int weekday : weekdays)
        for (const QTime& time : times)
            weekTimes.append({ weekday, time });
    return weekTimes;
}

QString parseAddress(hcxselect::Node* node) {
    const std::string text = node->first_child->data.text();
    const QString addrText = QString::fromStdString(text);
    int paren = addrText.lastIndexOf('(');
    return addrText.left(paren - 1);
}

void parseRow(const hcxselect::Selection& row) {
    const hcxselect::Selection cols = row.select("td:not([align])");
    const QString addr = parseAddress(*next(cols.begin()));
    if (addr == "TBA") return;
    const QList<WeekTime> weekTimes = parseWeekTimes(*cols.begin());
    qt_noop();
}
} // anonymous namespace

static const QString indexUrl = "https://w5.ab.ust.hk/wcq/cgi-bin/1810/";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new HttpClient(this))
{
    ui->setupUi(this);
    connect(client, &HttpClient::finished,
            this, &MainWindow::parseHtml);
    if (!getHtmlDir().exists())
        client->downloadAll(indexUrl);
    else parseHtml();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::parseHtml() {
    QDir htmlDir = getHtmlDir();
    htmlDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList entries = htmlDir.entryInfoList();
    for (QFileInfo entry : entries) {
        QFile file(entry.filePath());
        file.open(QFile::ReadOnly | QFile::Text);
        const std::string html = file.readAll().toStdString();
        HTML::ParserDom parser;
        const tree<HTML::Node>& dom = parser.parseTree(html);
        const std::string query = "table.sections tr:not(:first-child)";
        hcxselect::Selection rows(dom, query);
        for (hcxselect::Node* row : rows)
            parseRow(row);
    }
}
