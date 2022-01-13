#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
// should be 3 below
#define MINIMAL_NUMBER_OF_PLAYERS 1
#define IMPOSTOR_NUMBER 1

#define X_SPAWN 5500
#define Y_SPAWN 1100
#define X_SPAWN_POLUS 3500
#define Y_SPAWN_POLUS 3000

class Player {
public:
    int x, y;
    int bodyX, bodyY; // Only used if player has died: location of the dead body
    QString nickname, privateRandomHashed;
    bool playerFacingLeft;
    bool isImpostor;
    bool isGhost;
    bool showBody;
    bool isReady;
    bool isInvisible; // could make an enum to be cleaner
    quint8 numberOfEmergenciesRequested;
    QColor color1, color2;
    bool isPolus;

    Player();
    Player(QString nickname, bool Polus);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;
    QPixmap* deadPixmap; // no flippedDead ?
    QPixmap* ghostPixmap;
    QPixmap* flippedGhostPixmap;
    QPixmap* iconOnMapPixmap;

    QString getSendPositionMessage();
    void sendPosition();
    void moveTo(int x, int y);
};

#endif
