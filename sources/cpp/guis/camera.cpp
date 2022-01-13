#include "../main.h"

// 2640, 1760
// SPAWN 5500, 1100
#define CAMERA_X 0
#define CAMERA_Y 0

#define CAMERA_WIDTH 1120
#define CAMERA_HEIGHT 1120
// this isn't true for all cameras...

QVector<QPoint> cameraCenters{QPoint(6980, 2155), QPoint(4895, 2455), QPoint(3110, 940), QPoint(1920, 2275)};

quint16 cameraTileWidth, cameraTileHeight;
QPixmap* qBackgroundPixmap;

bool isNearCamera()
{
    quint32 dist = qPow(inGameUI->currPlayer.y - CAMERA_Y, 2) + qPow(inGameUI->currPlayer.x - CAMERA_X, 2);
    return dist < qPow(200, 2);
}

QLabel* getCamera()
{
    // should patch other interface too in order not to force redim the window
    cameraTileWidth = inGameUI->width() / 3;
    cameraTileHeight = inGameUI->height() / 3;
    QLabel* qLabel = new QLabel;
    qBackgroundPixmap = getQPixmap(cameraTileWidth * 2, cameraTileHeight * 2);
    QPainter* painter = new QPainter(qBackgroundPixmap);
    painter->fillRect(0, 0, cameraTileWidth * 2, cameraTileHeight * 2, Qt::red);
    for(quint8 y = 0; y < 2; y++)
        for(quint8 x = 0; x < 2; x++)
        {
            QPoint cameraCenter = cameraCenters[y * 2 + x];
            inGameUI->setCenterBorderLimit(cameraCenter.x(), cameraCenter.y(), painter, QSize(/*cameraTileWidth, cameraTileHeight*/CAMERA_WIDTH, CAMERA_HEIGHT), x * cameraTileWidth, y * cameraTileHeight, cameraTileWidth, cameraTileHeight);
        }
    qLabel->setPixmap(*qBackgroundPixmap);
    delete painter;

    return qLabel;
}

/*void replaceCameraPixmap(QPixmap* pixmap) {
    if(currAsteroidsLabel)
        currAsteroidsLabel->setPixmap(*pixmap);
    if(currAsteroidsPixmap)
        delete currAsteroidsPixmap;
    currAsteroidsPixmap = pixmap;
}*/

void redrawCamera()
{
    // could make 4 map tiles as background likewise just have to draw in real time doors and players
    for(quint8 y = 0; y < 2; y++)
        for(quint8 x = 0; x < 2; x++)
        {
            //drawDoors();
            //drawPlayers();
        }

    /*QPair<QPixmap*, QPainter*> pixmapPainter = getCameraPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    int fontSizePt = 23;
    painter->setFont(QFont("Liberation Sans", fontSizePt));
    painter->setPen(Qt::white);
    painter->drawText(0, 13*fontSizePt/10, "hey");

    delete painter;
    replaceCameraPixmap(pixmap);*/
}
