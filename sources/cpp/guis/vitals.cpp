#include "../main.h"

#define VITALS_X 5820
#define VITALS_Y 2480

QPixmap* qVitalsBackgroundPixmap, *qDefaultPlayer; // why can't be named qBackgroundPixmap ?
QLabel* qLabel;
QImage qPlayerBackground, qPlayerDeadBackground;

bool isNearVitals()
{
    quint32 dist = qPow(inGameUI->currPlayer.y - VITALS_Y, 2) + qPow(inGameUI->currPlayer.x - VITALS_X, 2);
    return dist < qPow(200, 2);
}

QLabel* getVitals()
{
    qLabel = new QLabel;
    qVitalsBackgroundPixmap = getQPixmap("vitals_background.png");
    qPlayerBackground = getQImage("vitals_player_background.png");
    qPlayerDeadBackground = colorImage(qPlayerBackground, QColor(0, 255, 0), QColor(255, 0, 0), QColor(255, 0, 0)).toImage();
    qDefaultPlayer = getQPixmap("vitals_player.png");
    qLabel->setPixmap(*qVitalsBackgroundPixmap);
    return qLabel;
}

void redrawVitals()
{
    QPixmap qVitalsPixmap = qVitalsBackgroundPixmap->copy();
    QPainter* painter = new QPainter(&qVitalsPixmap);
    QList<Player*> players = inGameUI->getAllPlayers();
    quint8 playersSize = players.size();
    for(quint8 playersIndex = 0; playersIndex < playersSize; playersIndex++)
    {
        Player* player = players[playersIndex];
        quint16 offset = 90 * playersIndex;
        painter->drawImage(QPoint(60 + offset, 95), player->isGhost && player->showBody ? qPlayerDeadBackground : qPlayerBackground);
        QImage qPlayer = colorPixmap(*qDefaultPlayer, player->color2, player->color1).toImage(); // why have to reverse here it's not logical
        painter->drawImage(QPoint(70 + offset, 534), qPlayer);
        QString state = player->isGhost ? (player->showBody ? QObject::tr("DEAD") : QObject::tr("D/C")) : QObject::tr("OK");
        painter->drawText(QPoint(85 + offset, 505), state);
        // could also display as a label the color of the player
    }
    qLabel->setPixmap(qVitalsPixmap);
    delete painter;
}
