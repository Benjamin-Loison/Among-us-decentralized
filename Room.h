#ifndef ROOM_H
#define ROOM_H

#include <QPoint>
#include <QString>
#include <QVector>
#include "Door.h"

class Door;

const int DOOR_SABOTAGE_COOLDOWN_SECS = 30;

struct Room {
    QString roomName;
    quint8 id;
    // where the door closure button will be drawn if required, else where
    // the room label will be centered
    QPoint roomCenter;
    QVector<Door*> doors;
    qint64 lastDoorSabotage = -DOOR_SABOTAGE_COOLDOWN_SECS*1000;

    bool isCoolingDown();
    void sabotage(); // no checks performed
};

#endif