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
    qint64 lastDoorSabotage;

    bool isCoolingDown();
    void sabotage(); // no checks performed

    Room();
    Room(const QString &roomName, const quint8 &id, const QPoint &roomCenter, const QVector<Door*> &doors);
};

#endif