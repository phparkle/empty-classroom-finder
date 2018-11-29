#include "emptyclassroomfinder.h"
#include "global.h"
#include "httpclient.h"

#include <hcxselect/hcxselect.h>
#include <htmlcxx/html/ParserDom.h>

#include <QDate>
#include <QDirIterator>
#include <QFile>
#include <QTime>

#include <string>

using namespace htmlcxx;

namespace {
int halfHour(const QTime& time) {
    return time.msecsSinceStartOfDay() / (1000 * 60 * 30);
}

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
    if (node->first_child != node->last_child) {
        const std::string text = node->first_child->data.text();
        const QString dateText = QString::fromStdString(text);
        if (!isTodayInRange(dateText))
            return weekTimes;
    }
    const hcxselect::Node* weekTimeNode = node->last_child;
    const std::string text = weekTimeNode->data.text();
    const QString weekTimeText = QString::fromStdString(text);
    const QList<int> weekdays = parseWeekdays(weekTimeText);
    const QList<QTime> times = parseTimes(weekTimeText);
    for (int weekday : weekdays)
        for (const QTime& time : times)
            weekTimes.append({ weekday, time });
    return weekTimes;
}

Address parseAddress(hcxselect::Node* node) {
    const std::string text = node->first_child->data.text();
    QString roomText = QString::fromStdString(text);
    const int paren = roomText.lastIndexOf('(');
    roomText = roomText.left(paren - 1);
    const int comma = roomText.indexOf(',');
    QString name = roomText.left(comma);
    QString area = comma == -1 ? "" : roomText.mid(comma + 2);
    return { name, area };
}

} // anonymous namespace

EmptyClassroomFinder::EmptyClassroomFinder(QObject* parent) :
    QObject(parent)
{
}

QSet<const Address*> EmptyClassroomFinder::findEmptyRooms(int weekday, QTime startTime, QTime endTime) {
    QSet<const Address*> rooms;
    const int start = halfHour(startTime);
    const int end = halfHour(endTime);
    for (int i = start; i < end; ++i)
        rooms += emptyRoomLists[weekday][i];
    auto it = rooms.begin();
    while (it != rooms.end()) {
        bool covers = true;
        for (int i = start; covers && i < end; ++i)
            if (!emptyRoomLists[weekday][i].contains(*it))
                covers = false;
        it = covers ? it + 1 : rooms.erase(it);
    }
    return rooms;
}

void EmptyClassroomFinder::parseRow(const hcxselect::Selection& row) {
    const hcxselect::Selection cols = row.select("td:not([align])");
    const Address addr = parseAddress(*next(cols.begin()));
    if (addr.isTBA()) return;
    const QList<WeekTime> weekTimes = parseWeekTimes(*cols.begin());
    addressMap[addr].append(weekTimes);
}

void EmptyClassroomFinder::parseHtml() {
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
    for (auto it = addressMap.constBegin(); it != addressMap.constEnd(); ++it) {
        const Address* addr = &it.key();
        bool occupied[5][48]{};
        for (const WeekTime& wt : it.value()) {
            occupied[wt.weekday][halfHour(wt.time)] = true;
        }
        for (int weekday = 0; weekday < 5; ++weekday)
            for (int halfhour = 0; halfhour < 48; ++halfhour)
                if (!occupied[weekday][halfhour])
                    emptyRoomLists[weekday][halfhour].insert(addr);
    }
}
