#ifndef PLAYER_H
#define PLAYER_H
#include <QPixmap>
// should be 3 below
#define MINIMAL_NUMBER_OF_PLAYERS 2

class Player {
public:
    int x, y;
    int bodyX, bodyY; // Only used if player has died: location of the dead body
    QString nickname;
    bool playerFacingLeft;
    bool isImpostor;
    bool isGhost;
    bool showBody;
    bool isReady;

    Player();
    Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;
    QPixmap* deadPixmap;
    QPixmap* ghostPixmap;
    QPixmap* flippedGhostPixmap;

};

#endif // PLAYER_H
