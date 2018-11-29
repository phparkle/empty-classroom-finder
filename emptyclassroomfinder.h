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

    QSet<const Address*> findEmptyRooms(int weekday, QTime startTime, QTime endTime);

public slots:
    void parseHtml();

private:
    void parseRow(const hcxselect::Selection& row);

    QHash<Address, QList<WeekTime>> addressMap;
    QSet<const Address*> emptyRoomLists[5][48];
};

#endif // EMPTYCLASSROOMFINDER_H
