#ifndef PLAYER_H
#define PLAYER_H
#include <QPixmap>

class Player {
public:
    int x, y;
    QString nickname;

    Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2);

private:
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;

};

#endif // PLAYER_H
