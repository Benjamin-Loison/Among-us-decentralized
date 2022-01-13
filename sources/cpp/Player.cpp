#include "Player.h"
#include "qPlus.h"
#include "main.h"
#include "Server.h"

const QColor colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                             {QColor(20, 156, 20), QColor(8, 99, 64)},
                             {QColor(17, 43, 192), QColor(8, 19, 131)},
                             {QColor(102, 67, 27), QColor(87, 35, 21)},
                             {QColor(193, 17, 17), QColor(120, 8, 57)},
                             {QColor(62, 71, 78), QColor(30, 30, 38)},
                             {QColor(244, 244, 86), QColor(194, 134, 34)}};
                             // should add skins and a limit of 15 players (source: https://fr.wikipedia.org/wiki/Among_Us)

Player::Player(QString nickname):
    x(X_SPAWN),
    y(Y_SPAWN),
    bodyX(-1),
    bodyY(-1),
    nickname(nickname),
    privateRandomHashed(""),
    playerFacingLeft(false),
    isImpostor(false),
    isGhost(false),
    showBody(false),
    isReady(false),
    isInvisible(false),
	isMoving(false),
    numberOfEmergenciesRequested(0),
	startMoveAt(0)
{
    quint8 playersNumber = inGameUI->getPlayersNumber();
    color1 = colors[playersNumber][0];
    color2 = colors[playersNumber][1];
    playerPixmap = getQPixmap("player.png");
    deadPixmap = getQPixmap("corpse.png");
    ghostPixmap = getQPixmap("ghost.png");
    iconOnMapPixmap = getQPixmap("playerIconOnMap.png");
    *playerPixmap = colorPixmap(*playerPixmap, color1, color2);
    *deadPixmap = colorPixmap(*deadPixmap, color1, color2);
    *ghostPixmap = colorPixmap(*ghostPixmap, color1, color2);
    *iconOnMapPixmap = colorPixmap(*iconOnMapPixmap, color1, color2);
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
    flippedGhostPixmap = new QPixmap(ghostPixmap->transformed(QTransform().scale(-1,1)));
	char filename[100];
	for (int i = 0; i < ANIMATION_SIZE; i++) {
		sprintf(filename, "Walk%04d.png", i);
		walkAnimation[i] = getQPixmap(filename);
		walkAnimation[ANIMATION_SIZE+i] = new QPixmap(playerPixmap->transformed(QTransform().scale(-1, 1)));
	}
}

// shouldn't have to exist in theory no ? because we don't use it...
Player::Player(): x(0), y(0), nickname(""), playerFacingLeft(false), playerPixmap(getQPixmap("player.png")), deadPixmap(getQPixmap("corpse.png")), ghostPixmap(getQPixmap("ghost.png")) {
    flippedPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
    flippedGhostPixmap = new QPixmap(ghostPixmap->transformed(QTransform().scale(-1,1)));
}

QString Player::getSendPositionMessage()
{
    return "Position " + QString::number(x) + " " + QString::number(y);
}

void Player::sendPosition()
{
    sendToAll(getSendPositionMessage());
}

void Player::moveTo(int x, int y) {
    this->x = x;
    this->y = y;
	this->isMoving = true;
    if(this == &inGameUI->currPlayer)
        sendPosition();
}

QPixmap* Player::getAnimationFrame(bool flipped, quint64 time) {
	int offset = flipped ? 1 : 0;
	if (!this->isMoving) {
		return this->walkAnimation[offset*ANIMATION_SIZE];
	} else {
		int frameNumber = 1;  // TODO find a good frame number	
		return this->walkAnimation[frameNumber + offset*ANIMATION_SIZE];
	}
}
