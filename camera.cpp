#include "main.h"

#define CAMERA_X 2640
#define CAMERA_Y 1760

bool isNearCamera()
{
    quint32 dist = qPow(inGameUI->currPlayer.y - CAMERA_Y, 2) + qPow(inGameUI->currPlayer.x - CAMERA_X, 2);
    return dist < qPow(200, 2);
}

QLabel* getCamera(){

    QLabel* qLabel = new QLabel;


    return qLabel;

}
