#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QtMath>
#include <QPainterPath>
#include <QString>
#include <QtDebug>
#include "AlignEngine.h"
#include "main.h"
#include "qPlus.h"




QPixmap* AlignEngineBackgroundPixmap = nullptr;
QPixmap* currAlignEnginePixmap = nullptr;
QLabel* currAlignEngineLabel = nullptr;



QPair<QPixmap*, QPainter*> getAlignEnginePixmapPainter()
{
    QPixmap* pixmap = new QPixmap(AlignEngineBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, AlignEngineBackgroundPixmap->toImage());

    return qMakePair(pixmap,painter);
}



QLabel* getAlignEngine(){
    if(!AlignEngineBackgroundPixmap) {AlignEngineBackgroundPixmap = getQPixmap("Align_Engine_Output_clean.png");};

    playSound("fix_wiring_task_open.wav");
    QLabel* qLabel = new QLabel;

    
    QPair<QPixmap*, QPainter*> pixmapPainter = getAlignEnginePixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    delete painter;
    qLabel->setPixmap(*pixmap);

    if(currAlignEnginePixmap) {
        delete currAlignEnginePixmap;
        currAlignEnginePixmap = nullptr;
    }
    currAlignEnginePixmap = pixmap;

    if(currAlignEngineLabel) {
        delete currAlignEngineLabel;
        currAlignEngineLabel = nullptr;
    }
    currAlignEngineLabel = qLabel;

    return qLabel;

} 



void onCloseAlignEngine() {
    playSound("fix_wiring_task_close.wav");
    if(currAlignEngineLabel)
        currAlignEngineLabel = nullptr;
    if(currAlignEnginePixmap)
        currAlignEnginePixmap = nullptr;
}
