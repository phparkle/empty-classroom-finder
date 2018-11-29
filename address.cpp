#include "address.h"

#include <QHash>

Address::Address(QString room, QString area) :
    room(room), area(area)
{
}

const QString& Address::getRoom() const {
    return room;
}

const QString& Address::getArea() const {
    return area;
}

bool Address::isTBA() const {
    return room == "TBA";
}

bool operator==(const Address& a, const Address& b) {
    return a.room == b.room && a.area == b.area;
}

uint qHash(const Address& key, uint seed) {
    return qHash(key.room, seed) ^ qHash(key.area, seed);
}
