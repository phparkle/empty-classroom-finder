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

// Returns the number of half-hours elapsed from midnight to the given time
int halfHour(const QTime& time) {
    return time.msecsSinceStartOfDay() / (1000 * 60 * 30);
}

// Returns a list of times in half-hour intervals from the given text
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

// Returns a list of weekdays from the text with 0 as Monday
QList<int> parseWeekdays(const QString& weekTimeText) {
    QList<int> daysOfWeek;
    static const QStringList labels = { "Mo", "Tu", "We", "Th", "Fr" };
    for (int i = 0; i < 5; ++i)
        if (weekTimeText.contains(labels[i]))
            daysOfWeek.append(i);
    return daysOfWeek;
}

// Returns true if today falls within the date range in the text
bool isTodayInRange(QString dateText) {
    const QString dateText1 = dateText.left(11);
    const QString dateText2 = dateText.right(11);
    static const QString format = "dd-MMM-YYYY";
    const QDate d1 = QDate::fromString(dateText1, format);
    const QDate d2 = QDate::fromString(dateText2, format);
    const QDate today = QDate::currentDate();
    return today >= d1 && today <= d2;
}

// Returns a list of WeekTime objects parsed from the given table column node
QList<WeekTime> parseWeekTimes(hcxselect::Node* node) {
    QList<WeekTime> weekTimes;

    // Check if there is a date range
    if (node->first_child != node->last_child) {
        const std::string text = node->first_child->data.text();
        const QString dateText = QString::fromStdString(text);
        // If today is not in the date range, ignore these timeslots
        if (!isTodayInRange(dateText))
            return weekTimes;
    }

    // Extract weekdays and times from the text
    const hcxselect::Node* weekTimeNode = node->last_child;
    const std::string text = weekTimeNode->data.text();
    const QString weekTimeText = QString::fromStdString(text);
    const QList<int> weekdays = parseWeekdays(weekTimeText);
    const QList<QTime> times = parseTimes(weekTimeText);

    // Add a weektime for each occupied half-hour period in the week
    for (int weekday : weekdays)
        for (const QTime& time : times)
            weekTimes.append({ weekday, time });
    return weekTimes;
}

// Returns an address object parsed from the given table column node
Address parseAddress(hcxselect::Node* node) {
    // Split the text into room number and area
    const std::string text = node->first_child->data.text();
    QString roomText = QString::fromStdString(text);
    const int paren = roomText.lastIndexOf('(');
    roomText = roomText.left(paren - 1);
    const int comma = roomText.indexOf(',');
    QString name = roomText.left(comma);
    QString area = comma == -1 ? "Other" : roomText.mid(comma + 2);
    return { name, area };
}

} // anonymous namespace

EmptyClassroomFinder::EmptyClassroomFinder(QObject* parent) :
    QObject(parent)
{
}

QSet<const Address*> EmptyClassroomFinder::findEmptyRooms(int weekday, QTime startTime, QTime endTime) {
    // Ensure startTime is earlier than endTime
    if (endTime < startTime)
        endTime = QTime(23, 30);

    // Start with an empty set
    QSet<const Address*> rooms;

    // Convert times to half-hours since midnight
    const int start = halfHour(startTime);
    const int end = halfHour(endTime);

    // Compute the union of all the addresses in the time range
    for (int i = start; i < end; ++i)
        rooms += emptyRoomLists[weekday][i];

    // Discard addresses that don't completely cover the time range
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
    // Select all table columns without "align" attribute
    const hcxselect::Selection cols = row.select("td:not([align])");

    // Parse the second result as an address and return if it is TBA
    const Address addr = parseAddress(*next(cols.begin()));
    if (addr.isTBA()) return;

    // Parse the first result as a time of week and add it to addrMap
    const QList<WeekTime> weekTimes = parseWeekTimes(*cols.begin());
    addressMap[addr].append(weekTimes);
}

void EmptyClassroomFinder::processHtml() {
    emit statusChanged(QString("Building index"));

    // Iterate through all files in the html directory
    QDir htmlDir = getHtmlDir();
    htmlDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList entries = htmlDir.entryInfoList();
    for (QFileInfo entry : entries) {
        // Read the entire file into memory
        QFile file(entry.filePath());
        file.open(QFile::ReadOnly | QFile::Text);
        const std::string html = file.readAll().toStdString();

        // Construct a DOM tree from HTML source
        HTML::ParserDom parser;
        const tree<HTML::Node>& dom = parser.parseTree(html);

        // Select and parse all table rows except headings
        const std::string query = "table.sections tr:not(:first-child)";
        hcxselect::Selection rows(dom, query);
        for (hcxselect::Node* row : rows)
            parseRow(row);
    }

    // Iterate through all room addresses
    for (auto it = addressMap.constBegin(); it != addressMap.constEnd(); ++it) {
        const Address* addr = &it.key();

        // Assume all timeslots are unoccupied by default
        bool occupied[5][48]{};

        // For each occupied time of week, set occupied[weekday][time] to true
        for (const WeekTime& wt : it.value())
            occupied[wt.weekday][halfHour(wt.time)] = true;

        // For each unoccupied timeslot, add the address to emptyRoomLists
        for (int weekday = 0; weekday < 5; ++weekday)
            for (int halfhour = 0; halfhour < 48; ++halfhour)
                if (!occupied[weekday][halfhour])
                    emptyRoomLists[weekday][halfhour].insert(addr);
    }

    // Notify main thread that processing finished
    emit finished();
}
