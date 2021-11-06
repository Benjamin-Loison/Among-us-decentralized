#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QtMath>
#include "asteroids.h"
#include "main.h"
#include "qPlus.h"
qint16 asteroids[4][3],nbasteroids=2;

QPair<QPixmap*, QPainter*> getAsteroidsPixmapPainter()
{
    QPixmap* qBackgroundPixmap = getQPixmap(600,600);
    QPainter* painter = new QPainter(qBackgroundPixmap);
    return qMakePair(qBackgroundPixmap, painter);
}

QLabel* getAsteroids()
{
    //qInfo("a");
    playSound("Fix_Wiring_task_open_sound.wav");
    QLabel* qFrame = new QLabel;
    QHBoxLayout* hbox = new QHBoxLayout(qFrame);
    QLabel* qLabel = new QLabel(qFrame);
    hbox->addStretch();
    hbox->addWidget(qLabel);
    hbox->addStretch();
    qFrame->setLayout(hbox);
    QPair<QPixmap*, QPainter*> pixmapPainter = getAsteroidsPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    painter->end();
    qLabel->setPixmap(*qBackgroundPixmap);

    return qFrame;
}

void onMouseEventAsteroids(QMouseEvent* mouseEvent)
{
    QLabel* qImage = new QLabel;
    QPair<QPixmap*, QPainter*> pixmapPainter = getAsteroidsPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    quint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
            mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    if(mouseY<=pixmapSize.height() && mouseY<=32768 && mouseX<=pixmapSize.width() && mouseX<=32768 ){
        if(nbasteroids<=0){
        }
    }
}
