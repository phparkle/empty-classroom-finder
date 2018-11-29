#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>

class Address
{
public:
    Address(QString room, QString area);

    const QString& getRoom() const;
    const QString& getArea() const;
    bool isTBA() const;

    friend bool operator==(const Address& a, const Address& b);
    friend uint qHash(const Address& key, uint seed);

private:
    const QString room;
    const QString area;
};

#endif // ADDRESS_H
