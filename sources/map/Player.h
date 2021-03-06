#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
// should be 3 below
#define MINIMAL_NUMBER_OF_PLAYERS 1
#define IMPOSTOR_NUMBER 1

enum Map{MAP_THE_SKELD, MAP_POLUS};
extern QVector<QPair<quint16, quint16>> SPAWNS;

QString getMapName(Map map);
Map getMap(QString mapStr);
QVector<Map> getAllMaps();
QStringList getAllMapsStr(), getAllCleanMapsStr();

#define ANIMATION_SIZE 13
class Player {
public:
    int x, y;
    int bodyX, bodyY; // Only used if player has died: location of the dead body
    QString nickname, privateRandomHashed;
    bool playerFacingLeft;
    bool isImpostor;
    bool isGhost;
    bool showBody;
    bool isReady;
    bool isInvisible; // could make an enum to be cleaner
	bool isMoving;
    quint8 numberOfEmergenciesRequested;
    QColor color1, color2;
	quint64 startMoveAt;
    quint8 playerId;

    Player();
    Player(QString nickname);
    QPixmap* playerPixmap;
    QPixmap* flippedPixmap;
    QPixmap* deadPixmap; // no flippedDead ?
    QPixmap* ghostPixmap;
    QPixmap* flippedGhostPixmap;
    QPixmap* iconOnMapPixmap;
	QPixmap* walkAnimation[ANIMATION_SIZE*2];

    QString getSendPositionMessage();
    void sendPosition();
    void moveTo(int x, int y);
	QPixmap* getAnimationFrame(bool flipped, quint64 time);
};

#endif
