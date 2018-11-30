// Filename: weektime.h
//
// The WeekTime struct represents a specific time of the week.

#ifndef WEEKTIME_H
#define WEEKTIME_H

#include <QTime>

struct WeekTime {
	// The weekday, with 0 as Monday
    int weekday;

    // The time of day
    QTime time;
};

#endif // WEEKTIME_H
