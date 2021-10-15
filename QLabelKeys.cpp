#include "QLabelKeys.h"
#include "main.h"
#include <algorithm>
using namespace std;

QLabelKeys::QLabelKeys(QLabel* parent) : QLabel(parent), qLabel(nullptr)
{
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    windowPixmap = new QPixmap(size());
    x = 50;
    y = 0;
    display();
}

void QLabelKeys::display()
{
    QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)},
           colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                           {QColor(20, 156, 20), QColor(8, 99, 64)},
                           {QColor(17, 43, 192), QColor(8, 19, 131)},
                           {QColor(102, 67, 27), QColor(87, 35, 21)},
                           {QColor(193, 17, 17), QColor(120, 8, 57)},
                           {QColor(62, 71, 78), QColor(30, 30, 38)},
                           {QColor(244, 244, 86), QColor(194, 134, 34)}};
    playerPixmap = getQPixmap("player.png");
    backgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"

    QImage tmp = playerPixmap->toImage();
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

    *playerPixmap = QPixmap::fromImage(tmp);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void QLabelKeys::redraw() {
    qDebug() << "Current window size:" << size();
    qDebug() << "x: " << x << " -- y: " << y;

    QPixmap* oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(size());
    QPainter* painter = new QPainter(windowPixmap);
    int winWidth = size().width(), winHeight = size().height();
    int backWidth = backgroundPixmap->size().width(), backHeight = backgroundPixmap->size().height();
    bool isLeftBorder = x <= winWidth/2, isRightBorder = backWidth >= winWidth && x >= backWidth - winWidth/2;
    bool isTopBorder = y <= winHeight/2, isBottomBorder = backHeight >= winHeight && y >= backHeight - winHeight/2;
    int leftBackground, topBackground;
    if(isLeftBorder)
        leftBackground = 0;
    else if(isRightBorder)
        leftBackground = -(backWidth-winWidth);
    else
        leftBackground = -(x-winWidth/2);
    if(isTopBorder)
        topBackground = 0;
    else if(isBottomBorder)
        topBackground = -(backHeight-winHeight);
    else
        topBackground = -(y-winHeight/2);
    painter->drawPixmap(leftBackground, topBackground, *backgroundPixmap);
    QSize playerSize = playerPixmap->size();
    int xPlayerWin, yPlayerWin;
    if(isLeftBorder)
        xPlayerWin = x;
    else if(isRightBorder)
        xPlayerWin = winWidth - (backWidth - x);
    else
        xPlayerWin = winWidth/2;
    if(isTopBorder)
        yPlayerWin = y;
    else if(isBottomBorder)
        yPlayerWin = winHeight - (backHeight - y);
    else
        yPlayerWin = winHeight/2;
    painter->drawPixmap(xPlayerWin-playerSize.width()/2, yPlayerWin-playerSize.height()/2, *playerPixmap);

    delete painter;
    setPixmap(*windowPixmap);
    delete oldPixmap;
    if(qLabel != nullptr && layout() == nullptr)
    {
        QHBoxLayout* hBox = new QHBoxLayout;
        hBox->addWidget(qLabel); // crash here ?
        setLayout(hBox);
    }
}

void QLabelKeys::resizeEvent(QResizeEvent* ev) {
    redraw();
    QLabel::resizeEvent(ev);
}

bool QLabelKeys::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj)

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        switch(key->key())
        {
            case Qt::Key_Down:
                y+=10;
                break;
            case Qt::Key_Up:
                y-=10;
                break;
            case Qt::Key_Left:
                x-=10;
                break;
            case Qt::Key_Right:
                x+=10;
                break;
            // https://nerdschalk.com/among-us-keyboard-controls/
            case Qt::Key_E:
                if(qLabel == nullptr)
                {
                    currentInGameGUI = IN_GAME_GUI_FIX_WIRING;
                    qLabel = getFixWiring();
                }
                else
                {
                    currentInGameGUI = IN_GAME_GUI_NONE;
                    //qLabel = new QLabel;
                    //qLabel->clear();
                    //free(qLabel);
                }
                break;
        }

        redraw();

        return true;
    }
    else if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if(qLabel != nullptr)
        {
            if(currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
                onMouseEventFixWiring(mouseEvent);
        }
        return true;
    }
    return false;
}