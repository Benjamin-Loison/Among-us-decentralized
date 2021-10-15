#ifndef QLABELKEYS_H
#define QLABELKEYS_H

#include <QLabel>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QRandomGenerator>
#include "fixWiring.h"
#include "main.h"
#include "qPlus.h"

class QLabelKeys : public QLabel
{
    Q_OBJECT

    private:
        quint8 x, y;
        QLabel* qLabel; // working with a QWidget maybe clever because we almost everytime use layouts

    public:
        void display()
        {
            QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)};
            QColor colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                                   {QColor(20, 156, 20), QColor(8, 99, 64)},
                                   {QColor(17, 43, 192), QColor(8, 19, 131)},
                                   {QColor(102, 67, 27), QColor(87, 35, 21)},
                                   {QColor(193, 17, 17), QColor(120, 8, 57)},
                                   {QColor(62, 71, 78), QColor(30, 30, 38)},
                                   {QColor(244, 244, 86), QColor(194, 134, 34)}};
            QPixmap* qPixmap = getQPixmap("player.png"),
                   * qBackgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"

            QImage tmp = qPixmap->toImage();
            quint8 colorsIndex = 2; // s'accorder sur de l'aléatoire en début de partie serait bien où bijection pseudo (s'il y en a un) au skin ?

            for(quint16 y = 0; y < tmp.height(); y++)
            {
                for(quint16 x = 0; x < tmp.width(); x++)
                {
                    for(quint8 originalColorsIndex = 0; originalColorsIndex < 2; originalColorsIndex++)
                    {
                        if(tmp.pixelColor(x, y) == originalColors[originalColorsIndex])
                            tmp.setPixelColor(x, y, colors[colorsIndex][originalColorsIndex]);
                    }
                }
            }

            *qPixmap = QPixmap::fromImage(tmp);

            QPainter* painter = new QPainter(qBackgroundPixmap);
            painter->drawPixmap(x, y, *qPixmap);

            painter->end();
            setPixmap(*qBackgroundPixmap);
            if(qLabel != nullptr && layout() == nullptr)
            {
                QHBoxLayout* hBox = new QHBoxLayout;
                hBox->addWidget(qLabel);
                setLayout(hBox);
            }
        }

        QLabelKeys(QLabel* parent = 0) : QLabel(parent)
        {
            setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
            setWindowTitle("Among Us decentralized");

            x = 50;
            y = 0;
            display();
        }

    protected:
        bool eventFilter(QObject* obj, QEvent* event)
        {
            Q_UNUSED(obj)

            if(event->type() == QEvent::KeyPress)
            {
                QKeyEvent* key = static_cast<QKeyEvent*>(event);
                switch(key->key())
                {
                    case Qt::Key_Down:
                        y++;
                        break;
                    case Qt::Key_Up:
                        y--;
                        break;
                    case Qt::Key_Left:
                        x--;
                        break;
                    case Qt::Key_Right:
                        x++;
                        break;
                    // https://nerdschalk.com/among-us-keyboard-controls/
                    case Qt::Key_E:
                        if(qLabel == nullptr)
                        {
                            qLabel = getFixWiring();
                        }
                        else
                        {
                            //qLabel = new QLabel;
                            //qLabel->clear();
                            //free(qLabel);
                        }
                        break;
                }

                display();

                return true;
            }
            else if(event->type() == QEvent::MouseMove)
            {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

                if(qLabel != nullptr)
                {
                    QLabel* qImage = new QLabel;
                    quint8 range = qFloor(FIX_WIRING_DELTA_Y / 2);
                    QPair<QPixmap*, QPainter*> pixmapPainter = getFixWiringPixmapPainter();
                    QPixmap* qBackgroundPixmap = pixmapPainter.first;
                    QSize pixmapSize = qBackgroundPixmap->size(),
                          windowSize = size();
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
                    QHBoxLayout* hbox = (QHBoxLayout*)qLabel->layout();
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
                return true;
            }
            return false;
        }
};

#endif
