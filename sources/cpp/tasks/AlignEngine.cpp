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
#include "../main.h"
#include "../qPlus.h"

QPixmap* AlignEngineBackgroundPixmap = nullptr;
QPixmap* currAlignEnginePixmap = nullptr;
QLabel* currAlignEngineLabel = nullptr;

QPixmap* pattern = nullptr;
QPixmap* arrow = nullptr;

qreal theta;

QElapsedTimer timer;
int elapsedtime = 0;

QPair<QPixmap*, QPainter*> getAlignEnginePixmapPainter()
{
    QPixmap* pixmap = new QPixmap(AlignEngineBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, AlignEngineBackgroundPixmap->toImage());
    return qMakePair(pixmap,painter);
}

void drawAssets(QPainter* painter, qreal angle){
    painter->save();
    painter->translate(PATTERN_X_START + PATTERN_ANCHOR_X,PATTERN_Y_START + PATTERN_ANCHOR_Y);
    painter->rotate(angle);
    painter->drawImage(-PATTERN_ANCHOR_X,-PATTERN_ANCHOR_Y,pattern->toImage());
    painter->restore();
    painter->save();
    painter->translate(ARROW_X_START + ARROW_ANCHOR_X,ARROW_Y_START + ARROW_ANCHOR_Y);
    painter->rotate(angle);
    painter->drawImage(-ARROW_ANCHOR_X,-ARROW_ANCHOR_Y,arrow->toImage());
    painter->restore();
}

QLabel* getAlignEngine(){
    if(!AlignEngineBackgroundPixmap) AlignEngineBackgroundPixmap = getQPixmap("Align_Engine_Output_clean.png");

    playSound("fix_wiring_task_open");
    QLabel* qLabel = new QLabel;

    QPair<QPixmap*, QPainter*> pixmapPainter = getAlignEnginePixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    if(!pattern) pattern = getQPixmap("Engine_Pattern.png");
    if(!arrow) arrow = getQPixmap("Engine_Arrow.png");

    theta = DEFAULT_ANGLE;
    drawAssets(painter,theta);
    timer.start();

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

void drawGreenLines(QPainter* painter){
    painter->save();
    painter->fillRect(QRect(60,360,590,15),QColor(102,255,51));
    painter->fillRect(QRect(60,575,590,15),QColor(102,255,51));
    painter->restore();
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

    bool correct = false;
    if (mouseX>695 && mouseX < 900){

        theta = -qreal(mouseY-466)/20;
        qInfo() <<mouseX << mouseY;
        qInfo()<< theta;
        drawAssets(painter,theta);

        if ((theta>3)||(theta<-3))
            elapsedtime = timer.elapsed();
        else
            drawGreenLines(painter);

        if (timer.elapsed() - elapsedtime > 0/*3000*/ )
            correct = true;
    }
    else drawAssets(painter,theta);
    painter->end();

    if(currAlignEngineLabel)
        currAlignEngineLabel->setPixmap(*qBackgroundPixmap);
    if(currAlignEnginePixmap)
        delete currAlignEnginePixmap;
    currAlignEnginePixmap = qBackgroundPixmap;

    if(correct)
    {
        playSound("task_completed");
        inGameUI->finishTask();
        inGameUI->closeTask();
        inGameUI->checkEndOfTheGame();
    }
}

void onCloseAlignEngine() {
    playSound("fix_wiring_task_close");
    if(currAlignEngineLabel)
        currAlignEngineLabel = nullptr;
    if(currAlignEnginePixmap)
        currAlignEnginePixmap = nullptr;
}
