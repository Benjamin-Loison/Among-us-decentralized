#ifndef PLAYER_H
#define PLAYER_H
#include <QPixmap>

class Player {
public:
    int x, y;
    int bodyX, bodyY; // Only used if player has died: location of the dead body
    QString nickname;
    bool playerFacingLeft;
    bool isImpostor;
    bool isGhost;
    bool showBody;

    Player();
    Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;
    QPixmap* deadPixmap;

};

#endif // PLAYER_H
