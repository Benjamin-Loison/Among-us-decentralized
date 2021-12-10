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

QPixmap* pattern = nullptr;
QPixmap* arrow = nullptr;

qreal angle = 5;



QPair<QPixmap*, QPainter*> getAlignEnginePixmapPainter()
{
    QPixmap* pixmap = new QPixmap(AlignEngineBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, AlignEngineBackgroundPixmap->toImage());
    return qMakePair(pixmap,painter);
}


void rotatePattern(QPainter* painter, qreal angle){
    painter->translate(PATTERN_X_START + PATTERN_ANCHOR_X,PATTERN_Y_START + PATTERN_ANCHOR_Y);
    painter->rotate(angle);
    painter->drawImage(-PATTERN_ANCHOR_X,-PATTERN_ANCHOR_Y,pattern->toImage());
}


void rotateArrow(QPainter* painter, qreal angle){
    painter->translate(ARROW_X_START + ARROW_ANCHOR_X,ARROW_Y_START + ARROW_ANCHOR_Y);
    painter->rotate(angle);
    painter->drawImage(-ARROW_ANCHOR_X,-ARROW_ANCHOR_Y,arrow->toImage());
}


QLabel* getAlignEngine(){
    if(!AlignEngineBackgroundPixmap) {AlignEngineBackgroundPixmap = getQPixmap("Align_Engine_Output_clean.png");};

    playSound("fix_wiring_task_open.wav");
    QLabel* qLabel = new QLabel;

    
    QPair<QPixmap*, QPainter*> pixmapPainter = getAlignEnginePixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    if(!pattern) {pattern = getQPixmap("Engine_Pattern.png");};
    if(!arrow) {arrow = getQPixmap("Engine_Arrow.png");};

    rotatePattern(painter,0);
    rotateArrow(painter,0);

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

void onMouseEventAlignEngine(QMouseEvent* mouseEvent)
{   
    QPair<QPixmap*, QPainter*> pixmapPainter = getAlignEnginePixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    if(mouseX < 0 || mouseX >= pixmapSize.width() || mouseY < 0 || mouseY >= pixmapSize.height())
    {
        qInfo("get out");
        return;
    }
    qInfo() << mouseX << mouseY ;
    angle = qreal(mouseY-466)/5;
    qInfo()<< angle;
    painter->save();
    rotateArrow(painter,angle);
    painter->restore();
    rotatePattern(painter,angle);
    painter->restore();
    painter->end();
}



void onCloseAlignEngine() {
    playSound("fix_wiring_task_close.wav");
    if(currAlignEngineLabel)
        currAlignEngineLabel = nullptr;
    if(currAlignEnginePixmap)
        currAlignEnginePixmap = nullptr;
}
