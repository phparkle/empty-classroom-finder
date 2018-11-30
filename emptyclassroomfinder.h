// Filename: emptyclassroomfinder.h
//
// The EmptyClassroomFinder class processes HTML files and handles queries for
// empty classrooms for a given weekday, start time and end time.

#ifndef EMPTYCLASSROOMFINDER_H
#define EMPTYCLASSROOMFINDER_H

#include <QObject>
#include <QSet>

#include "address.h"
#include "weektime.h"

namespace hcxselect {
class Selection;
}

class EmptyClassroomFinder : public QObject
{
    Q_OBJECT

public:
    EmptyClassroomFinder(QObject *parent = nullptr);

    // Returns a set of addresses corresponding to the empty rooms on the given
    // weekday from startTime to endTime.
    QSet<const Address*> findEmptyRooms(int weekday, QTime startTime, QTime endTime);

signals:
	// Emitted when status changes
    void statusChanged(QString message);

    // Emitted when processHtml finishes
    void finished();

public slots:
	// Processes html and populates data members with results
    void processHtml();

private:
	// Parses a single table row from an HTML file
    void parseRow(const hcxselect::Selection& row);

    // The list of occupied times per address
    QHash<Address, QList<WeekTime>> addressMap;

    // An array of sets of addresses corresponding to empty rooms per half-hour
    // per weekday, where the first array dimension is the weekday and the
    // second array dimension is the number of half-hours elapsed since midnight
    QSet<const Address*> emptyRoomLists[5][48];
};

#endif // EMPTYCLASSROOMFINDER_H
