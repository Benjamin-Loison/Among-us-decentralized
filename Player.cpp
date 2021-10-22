#include "Player.h"
#include "qPlus.h"
#include <QDebug>

Player::Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2): x(spawnX), y(spawnY), nickname(nickname), playerFacingLeft(false) {
    QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)};
    QColor colors[2] = {color1, color2};
    playerPixmap = getQPixmap("player.png");

    QImage tmp = playerPixmap->toImage();

    for(quint16 y = 0; y < tmp.height(); y++)
    {
        for(quint16 x = 0; x < tmp.width(); x++)
        {
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 2; originalColorsIndex++)
            {
                if(tmp.pixelColor(x, y) == originalColors[originalColorsIndex])
                    tmp.setPixelColor(x, y, colors[originalColorsIndex]);
            }
        }
    }

    *playerPixmap = QPixmap::fromImage(tmp);
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
}

Player::Player(): x(0), y(0), nickname("Player"), playerPixmap(getQPixmap("player.png")), playerFacingLeft(false) {
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
}

Player::~Player() {
    /*delete playerPixmap;
    delete flippedPixmap;*/
}