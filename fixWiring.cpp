#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QtMath>
#include <QPainterPath>
#include "fixWiring.h"
#include "main.h"
#include "qPlus.h"

// à quoi bon utiliser un enum si juste 4 defines aurait fait la bijection sans warning du compilo u_u
quint8 links[COLORS_NUMBER] = {COLOR_UNDEFINED, COLOR_UNDEFINED, COLOR_UNDEFINED, COLOR_UNDEFINED};
WiringColor lefts[COLORS_NUMBER],
            rights[COLORS_NUMBER] = {Red, Blue, Yellow, Magenta};
QPixmap* fixWiringBackgroundPixmap = nullptr;
QPixmap* currFixWiringPixmap = nullptr;
QLabel* currFixWiringLabel = nullptr;

WiringColor getColor(quint8 colorIndex)
{
    switch(colorIndex)
    {
        case 0:
            return Yellow;
        case 1:
            return Blue;
        case 2:
            return Red;
        default: // 3
            return Magenta;
    }
}

quint8 getIndex(WiringColor color)
{
    switch(color)
    {
        case Yellow:
            return 0;
        case Blue:
            return 1;
        case Red:
            return 2;
        default: // magenta
            return 3;
    }
}

void randomWires(WiringColor* nodes)
{
    // could generalize a method to mix a QList which may be useful for other tasks
    for(quint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 randomColorIndex;
        do
        {
            randomColorIndex = QRandomGenerator::global()->bounded(COLORS_NUMBER);
            bool alreadyIn = false;
            for(quint8 subNodesIndex = 0; subNodesIndex < nodesIndex; subNodesIndex++)
            {
                if(getIndex(nodes[subNodesIndex]) == randomColorIndex)
                {
                    alreadyIn = true;
                    break;
                }
            }
            if(!alreadyIn)
                break;
        } while(true);
        nodes[nodesIndex] = getColor(randomColorIndex);
    }
}

QColor getQtColor(WiringColor color)
{
    switch(color)
    {
        case Yellow:
            return Qt::yellow;
        case Blue:
            return Qt::blue;
        case Red:
            return Qt::red;
        default: // magenta
            return Qt::magenta;
    }
}

quint16 getYForWiring(quint8 nodesIndex)
{
    return (nodesIndex == 0 ? -1 : 0) + FIX_WIRING_TOP_Y + FIX_WIRING_DELTA_Y * nodesIndex;
}

void fillFixWire(QPainter* painter, quint8 start, quint16 y, quint16 x)
{
    quint16 yLeft = getYForWiring(start),
            yRight = y;
    QPainterPath path;
    path.moveTo(FIX_WIRING_LEFT_X + FIX_WIRING_WIDTH, yLeft);
    path.lineTo(FIX_WIRING_LEFT_X + FIX_WIRING_WIDTH, yLeft + FIX_WIRING_HEIGHT);
    path.lineTo(x, yRight + FIX_WIRING_HEIGHT);
    path.lineTo(x, yRight);

    painter->setPen(Qt::NoPen);
    painter->fillPath(path, QBrush(getQtColor(lefts[start])));
}

void fillWire(QPainter* painter, quint8 start)
{
    quint8 link = links[start];
    if(link == COLORS_NUMBER + 1)
        return;
    quint16 yRight = getYForWiring(link);
    fillFixWire(painter, start, yRight, FIX_WIRING_RIGHT_X);
}

QPair<QPixmap*, QPainter*> getFixWiringPixmapPainter()
{
    QPixmap* pixmap = new QPixmap(fixWiringBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, fixWiringBackgroundPixmap->toImage());

    for(quint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint16 y = getYForWiring(nodesIndex);
        painter->fillRect(FIX_WIRING_LEFT_X, y, FIX_WIRING_WIDTH, FIX_WIRING_HEIGHT, getQtColor(lefts[nodesIndex])); // as QBrush Qt::SolidPattern, Qt::HorPattern looks cool too
        painter->fillRect(FIX_WIRING_RIGHT_X, y, FIX_WIRING_WIDTH, FIX_WIRING_HEIGHT, getQtColor(rights[nodesIndex]));
    }
    return qMakePair(pixmap, painter);
}

QLabel* getFixWiring()
{
    if(!fixWiringBackgroundPixmap)
        fixWiringBackgroundPixmap = getQPixmap("fixWiring.png");
    playSound("fix_wiring_task_open");
    QLabel* qLabel = new QLabel;

    // 2 * COLORS_NUMBER! possibilities
    randomWires(lefts);
    //randomWires(rights); // could almost directly change the background image for the right side color constants

    QPair<QPixmap*, QPainter*> pixmapPainter = getFixWiringPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    delete painter;
    qLabel->setPixmap(*pixmap);

    if(currFixWiringPixmap) {
        delete currFixWiringPixmap;
        currFixWiringPixmap = nullptr;
    }
    currFixWiringPixmap = pixmap;

    if(currFixWiringLabel) {
        delete currFixWiringLabel;
        currFixWiringLabel = nullptr;
    }
    currFixWiringLabel = qLabel;

    return qLabel;
}

void onMouseEventFixWiring(QMouseEvent* mouseEvent)
{
    quint8 range = qFloor(FIX_WIRING_DELTA_Y / 2);
    QPair<QPixmap*, QPainter*> pixmapPainter = getFixWiringPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    if(mouseX < 0 || mouseX >= pixmapSize.width() || mouseY < 0 || mouseY >= pixmapSize.height())
        return;

    bool isFixing = false;
    quint8 fixingIndex = 0;
    for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 link = links[nodesIndex];
        if(link == COLOR_FIXING)
        {
            isFixing = true;
            fixingIndex = nodesIndex;
            break;
        }
    }

    if(isFixing)
    {
        for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
        {
            quint16 middleX = FIX_WIRING_RIGHT_X + FIX_WIRING_WIDTH / 2,
                    middleY = getYForWiring(nodesIndex);
            double d = distance(middleX, middleY, mouseX, mouseY);
            if(d <= range)
            {
                links[fixingIndex] = nodesIndex;
                playSound(QString("fix_wiring_connect_wire_%1").arg(QRandomGenerator::global()->bounded(3)));
                break;
            }
        }
    }

    for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 link = links[nodesIndex];
        if(link < COLOR_UNDEFINED)
            fillWire(painter, nodesIndex);
        else if(link == COLOR_FIXING)
            fillFixWire(painter, nodesIndex, mouseY, mouseX);
        else if(!isFixing)
        {
            quint16 middleX = FIX_WIRING_LEFT_X + FIX_WIRING_WIDTH / 2,
                    middleY = getYForWiring(nodesIndex);
            double d = distance(middleX, middleY, mouseX, mouseY);
            if(d <= range)
            {
                links[nodesIndex] = COLOR_FIXING;
                nodesIndex--;
            }
        }
    }

    painter->end();

    if(currFixWiringLabel)
        currFixWiringLabel->setPixmap(*qBackgroundPixmap);
    if(currFixWiringPixmap)
        delete currFixWiringPixmap;
    currFixWiringPixmap = qBackgroundPixmap;

    bool everythingMatch = true;
    for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 link = links[nodesIndex];
        if(link >= COLORS_NUMBER) {
            everythingMatch = false;
            break;
        }
        quint8 right = getIndex(rights[link]);
        if(lefts[nodesIndex] != right)
        {
            everythingMatch = false;
            break;
        }
    }
    if(everythingMatch)
    {
        playSound("task_completed"); // shouldn't be in finishTask function ?
        inGameUI->finishTask();
        inGameUI->closeTask();
        inGameUI->checkEndOfTheGame();
    }
}

void onCloseFixWiring() {
    playSound("fix_wiring_task_close");
    resetFixWiring();
    if(currFixWiringLabel)
        currFixWiringLabel = nullptr;
    if(currFixWiringPixmap) {
        delete currFixWiringPixmap;
        currFixWiringPixmap = nullptr;
    }
}

void resetFixWiring() {
    for(quint8 i = 0; i < COLORS_NUMBER; i++)
        links[i] = COLOR_UNDEFINED;
}
