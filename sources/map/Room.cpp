#include <QtGlobal>
#include "../main.h"
#include "Room.h"
#include "../network/Server.h"

Room::Room():
    roomName(""),
    id(0),
    roomCenter(),
    doors(),
    lastDoorSabotage(-DOOR_SABOTAGE_COOLDOWN_MSECS) {}

Room::Room(const QString &roomName, const quint8 &id, const QPoint &roomCenter, const QVector<Door*> &doors) :
    roomName(roomName),
    id(id),
    roomCenter(roomCenter),
    doors(doors),
    lastDoorSabotage(-DOOR_SABOTAGE_COOLDOWN_MSECS) {}

bool Room::isCoolingDown() {
    return inGameUI->currTimer() - lastDoorSabotage < DOOR_SABOTAGE_COOLDOWN_MSECS;
}

void Room::sabotage() {
    for(Door* door : doors)
        door->close();
    /*qInfo("Sabotaged room %d. Door statuses:", id);
    for(Door &door : inGameUI->doors)
        qInfo() << "- Door at" << door.getHitbox() << (door.isClosed() ? "is closed" : "is not closed");*/
    lastDoorSabotage = inGameUI->currTimer();
}
