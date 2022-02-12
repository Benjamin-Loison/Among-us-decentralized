#include "Player.h"
#include "../qPlus.h"
#include "../main.h"
#include "../network/Server.h"

const QColor colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                             {QColor(20, 156, 20), QColor(8, 99, 64)},
                             {QColor(17, 43, 192), QColor(8, 19, 131)},
                             {QColor(102, 67, 27), QColor(87, 35, 21)},
                             {QColor(193, 17, 17), QColor(120, 8, 57)},
                             {QColor(62, 71, 78), QColor(30, 30, 38)},
                             {QColor(244, 244, 86), QColor(194, 134, 34)}};
                             // should add skins and a limit of 15 players (source: https://fr.wikipedia.org/wiki/Among_Us)

Player::Player(QString nickname, Map map):
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
    this->map = map;
	switch(map)
	{
		case MAP_THE_SKELD:
			x = X_SPAWN_THE_SKELD;
			y = Y_SPAWN_THE_SKELD;
			break;
		default: // MAP_POLUS
			x = X_SPAWN_POLUS;
			y = Y_SPAWN_POLUS;
	}
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
		sprintf(filename, "walk/Walk%04d.png", i);
		walkAnimation[i] = getQPixmap(filename);
		*walkAnimation[i] = colorPixmapAnim(*walkAnimation[i], QColor(0, 255, 0), color1, color2);
		walkAnimation[ANIMATION_SIZE+i] = new QPixmap(walkAnimation[i]->transformed(QTransform().scale(-1, 1)));
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
		int frameNumber = ((time-this->startMoveAt) / (1000/20)) % (ANIMATION_SIZE-1);
		this->isMoving = false;
		return this->walkAnimation[frameNumber +1 + offset*ANIMATION_SIZE];
	}
}

QVector<Map> getAllMaps()
{
	// could initialize this fixed value at the initialization of the program
	QVector<Map> maps;
	maps.push_back(MAP_THE_SKELD);
	maps.push_back(MAP_POLUS);
	return maps;
}

QStringList getAllMapsStr()
{
	QVector<Map> maps = getAllMaps();
	QStringList mapsStr;
	for(Map map : maps)
		mapsStr.append(getMapName(map));
	return mapsStr;
}

QString getCleanMapName(Map map)
{
	switch(map)
    {
        case MAP_THE_SKELD:
            return "The Skeld"; // could use macro if change their name later
        default: // MAP_POLUS
            return "Polus";
    }
}

QStringList getAllCleanMapsStr()
{
    QVector<Map> maps = getAllMaps();
    QStringList mapsStr;
    for(Map map : maps)
        mapsStr.append(getCleanMapName(map));
    return mapsStr;
}

QString toCamelCase(const QString& s)
{
    QStringList parts = s.split(' ', /*QString*/Qt::SkipEmptyParts);
	parts[0].replace(0, 1, parts[0][0].toLower());
    for(quint8 i = 1; i < parts.size(); i++)
        parts[i].replace(0, 1, parts[i][0].toUpper());
    return parts.join("");
}

QString getMapName(Map map)
{
	QString res = getCleanMapName(map);
	return toCamelCase(res); // assume no map name with triple (or more) parts
}

Map getMap(QString mapStr)
{
	if(mapStr == "The Skeld")
		return MAP_THE_SKELD;
	else // "Polus"
		return MAP_POLUS;
}