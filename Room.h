#ifndef ROOM_H
#define ROOM_H

#include <QPoint>
#include <QString>
#include <QVector>
#include "Door.h"

class Door;

struct Room {
    QString roomName;
    // where the door closure button will be drawn if required, else where
    // the room label will be centered
    QPoint roomCenter;
    QVector<Door*> doors;
};

#endif