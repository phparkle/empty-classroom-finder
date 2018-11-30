// Filename: address.h
//
// The Address class uniquely represents the "address" of a room by its room
// number and area.

#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>

class Address
{
public:
    // Constructs an address with the given room number and area.
    Address(QString room, QString area);

    // Returns the room number
    const QString& getRoom() const;

    // Returns the area
    const QString& getArea() const;

    // Returns true if the address is TBA
    bool isTBA() const;

    // Returns true if a and b are equal
    friend bool operator==(const Address& a, const Address& b);

    // Returns a hashcode for this object
    friend uint qHash(const Address& key, uint seed);

private:
    // The room
    const QString room;

    // The area
    const QString area;
};

#endif // ADDRESS_H
