#include "Player.h"
#include "qPlus.h"
#include <QDebug>

Player::Player(int spawnX, int spawnY, QString nickname, QColor color1, QColor color2):
    x(spawnX),
    y(spawnY),
    bodyX(-1),
    bodyY(-1),
    nickname(nickname),
    playerFacingLeft(false),
    isImpostor(false),
    isGhost(false),
    showBody(false) {
    playerPixmap = getQPixmap("player.png");
    deadPixmap = getQPixmap("Dead0042.png");
    ghostPixmap = getQPixmap("ghost0001.png");
    *playerPixmap = colorPixmap(*playerPixmap, color1, color2);
    *deadPixmap = colorPixmap(*deadPixmap, color1, color2);
    *ghostPixmap = colorPixmap(*ghostPixmap, color1, color2);
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
    flippedGhostPixmap = new QPixmap(ghostPixmap->transformed(QTransform().scale(-1,1)));
}

Player::Player(): x(0), y(0), nickname("Player"), playerFacingLeft(false), playerPixmap(getQPixmap("player.png")), deadPixmap(getQPixmap("Dead0042.png")), ghostPixmap(getQPixmap("ghost0001.png")) {
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
    flippedGhostPixmap = new QPixmap(ghostPixmap->transformed(QTransform().scale(-1,1)));
}

