#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QtMath>
#include "fixWiring.h"
#include "main.h"
#include "qPlus.h"

// à quoi bon utiliser un enum si juste 4 defines aurait fait la bijection sans warning du compilo u_u
quint8 links[COLORS_NUMBER] = {COLOR_UNDEFINED, COLOR_UNDEFINED, COLOR_UNDEFINED, COLOR_UNDEFINED};
WiringColor lefts[COLORS_NUMBER],
            rights[COLORS_NUMBER] = {Red, Blue, Yellow, Magenta};

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
    QPixmap* qBackgroundPixmap = getQPixmap("fixWiring.png");
    QPainter* painter = new QPainter(qBackgroundPixmap);

    for(quint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint16 y = getYForWiring(nodesIndex);
        painter->fillRect(FIX_WIRING_LEFT_X, y, FIX_WIRING_WIDTH, FIX_WIRING_HEIGHT, getQtColor(lefts[nodesIndex])); // as QBrush Qt::SolidPattern, Qt::HorPattern looks cool too
        painter->fillRect(FIX_WIRING_RIGHT_X, y, FIX_WIRING_WIDTH, FIX_WIRING_HEIGHT, getQtColor(rights[nodesIndex]));
    }
    return qMakePair(qBackgroundPixmap, painter);
}

QLabel* getFixWiring()
{
    //qInfo("a");
    playSound("Fix_Wiring_task_open_sound.ogg");
    QLabel* qFrame = new QLabel,
          * qLabel = new QLabel;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(qLabel);
    hbox->addStretch();
    qFrame->setLayout(hbox);

    // 2 * COLORS_NUMBER! possibilities
    randomWires(lefts);
    //randomWires(rights); // could almost directly change the background image for the right side color constants

    QPair<QPixmap*, QPainter*> pixmapPainter = getFixWiringPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    painter->end();
    qLabel->setPixmap(*qBackgroundPixmap);

    return qFrame;
}

void onMouseEvent(QMouseEvent* mouseEvent)
{
    QLabel* qImage = new QLabel;
    quint8 range = qFloor(FIX_WIRING_DELTA_Y / 2);
    QPair<QPixmap*, QPainter*> pixmapPainter = getFixWiringPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = qLabelKeys->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    quint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
            mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;

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
                playSound("Fix_Wiring_connect_wire_sound_" + QString::number(QRandomGenerator::global()->bounded(3) + 1) + ".ogg");
                break;
            }
        }
    }

    for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 link = links[nodesIndex];
        if(link < COLOR_UNDEFINED)
        {
            fillWire(painter, nodesIndex);
        }
        else if(link == COLOR_FIXING)
        {
            fillFixWire(painter, nodesIndex, mouseY, mouseX);
        }
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

    qImage->setPixmap(*qBackgroundPixmap);
    QHBoxLayout* hbox = (QHBoxLayout*)qLabelKeys->qLabel->layout();
    hbox->takeAt(1);
    hbox->takeAt(1);
    hbox->addWidget(qImage);
    hbox->addStretch();
    /* sometimes this happen when launching (maybe due to huge image)
     * 21:47:13: The program has unexpectedly finished.
        21:47:13: The process was ended forcefully.
        */

    bool everythingMatch = true;
    for(qint8 nodesIndex = 0; nodesIndex < COLORS_NUMBER; nodesIndex++)
    {
        quint8 link = links[nodesIndex],
               right = getIndex(rights[link]);
        if(lefts[nodesIndex] != right)
        {
            everythingMatch = false;
            break;
        }
    }
    if(everythingMatch)
    {
        playSound("Fix_Wiring_task_close_sound.ogg");
    }
}
