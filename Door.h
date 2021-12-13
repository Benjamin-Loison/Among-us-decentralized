#ifndef DOORS_H
#define DOORS_H

#include <QtGlobal>
#include "InGameUI.h"
#include "Player.h"
#include <QRect>

/**
 * Stores information about a door that can be closed by Impostors, and reopens
 * automatically after a fixed delay.
 */
class Door {
    QRect hitbox;
    QPoint drawPoint; // top left position of the door sprite
    bool isVertical;
    qint64 lastClosed;

public:
    bool isClosed();
    void close();
    bool collidesWithPlayer(Player *player);
    void ejectPlayer(Player *player);
    void draw(QPainter *painter, int leftBackground, int topBackground);
    Door();
    Door(int x, int y, bool isVertical); // top left position of the sprite
};

#endif