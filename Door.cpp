#include "Door.h"
#include "main.h"
#include "qPlus.h"
#include <QRect>

const int DOOR_CLOSURE_DURATION_SECS = 10;
const QRect DOOR_HITBOX[2] = { // 0: horizontal, 1: vertical
    QRect(3, 153, 280, 53), // left, top, width, height
    QRect(4, 139, 53, 212)
};

QPixmap* doorPixmap[2] = {nullptr, nullptr};

Door::Door() : hitbox(), drawPoint(), isVertical(false), lastClosed(-(DOOR_CLOSURE_DURATION_SECS+1)*(qint64)1000) {}

Door::Door(int x, int y, bool isVertical) :
    hitbox(QPoint(x+DOOR_HITBOX[isVertical].x(), y+DOOR_HITBOX[isVertical].y()), DOOR_HITBOX[isVertical].size()),
    drawPoint(x, y),
    isVertical(isVertical), lastClosed(0) {
    if(!doorPixmap[isVertical]) {
        if(isVertical)
            doorPixmap[isVertical] = getQPixmap("doorSideClosed.png");
        else
            doorPixmap[isVertical] = getQPixmap("doorFrontClosed.png");
    }
    lastClosed = -(DOOR_CLOSURE_DURATION_SECS+1)*(qint64)1000;
}

QRect Door::getHitbox() {
    return hitbox;
}

bool Door::isClosed() {
    qint64 delay = inGameUI->currTimer() - lastClosed;
    return delay < DOOR_CLOSURE_DURATION_SECS * (qint64)1000;
}

void Door::close() {
    lastClosed = inGameUI->currTimer();
    if(collidesWithPosition(inGameUI->currPlayer.x, inGameUI->currPlayer.y))
        ejectPlayer(&(inGameUI->currPlayer));
}

bool Door::collidesWithPosition(int x, int y) {
    return isClosed() && hitbox.contains(x, y);
}

void Door::ejectPlayer(Player *player) {
    if(isVertical) {
        int xFromDoorLeft = player->x - hitbox.left();
        if(xFromDoorLeft >= 0 && xFromDoorLeft < hitbox.width()/2)
            player->moveTo(hitbox.left()-1, player->y);
        else if(xFromDoorLeft >= hitbox.width()/2 && xFromDoorLeft < hitbox.width())
            player->moveTo(hitbox.right(), player->y);
    }
    else {
        int yFromDoorTop = player->y - hitbox.top();
        if(yFromDoorTop >= 0 && yFromDoorTop < hitbox.height()/2)
            player->moveTo(player->x, hitbox.top()-1);
        else if(yFromDoorTop >= hitbox.height()/2 && yFromDoorTop < hitbox.height())
            player->moveTo(player->x, hitbox.bottom());
    }
}

void Door::draw(QPainter *painter, int leftBackground, int topBackground) {
    if(isClosed())
        painter->drawPixmap(leftBackground + drawPoint.x(), topBackground + drawPoint.y(), *doorPixmap[isVertical]);
}