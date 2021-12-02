#ifndef PLAYER_H
#define PLAYER_H
#include <QPixmap>
// should be 3 below
#define MINIMAL_NUMBER_OF_PLAYERS 1
#define IMPOSTOR_NUMBER 1

#define X_SPAWN 5500
#define Y_SPAWN 1100

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
    QColor color1, color2;

    Player();
    Player(QString nickname);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;
    QPixmap* deadPixmap; // no flippedDead ?
    QPixmap* ghostPixmap;
    QPixmap* flippedGhostPixmap;
    QPixmap* iconOnMapPixmap;

};

#endif // PLAYER_H
