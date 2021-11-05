#ifndef PLAYER_H
#define PLAYER_H
#include <QPixmap>

class Player {
public:
    int x, y;
    QString nickname;
    bool playerFacingLeft;

    Player();
    Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;

};

#endif // PLAYER_H
