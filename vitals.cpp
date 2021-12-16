#include "main.h"

// 5820, 2480
#define VITALS_X 5500
#define VITALS_Y 1100

QPixmap* qVitalsBackgroundPixmap; // why can't be named qBackgroundPixmap ?

bool isNearVitals()
{
    quint32 dist = qPow(inGameUI->currPlayer.y - VITALS_Y, 2) + qPow(inGameUI->currPlayer.x - VITALS_X, 2);
    return dist < qPow(200, 2);
}

QLabel* getVitals()
{
    QLabel* qLabel = new QLabel;
    qVitalsBackgroundPixmap = getQPixmap("vitals_background.png");;
    qLabel->setPixmap(*qVitalsBackgroundPixmap);
    return qLabel;
}

void redrawVitals()
{

}
