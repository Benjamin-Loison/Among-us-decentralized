#include "QLabelKeys.h"
#include "main.h"
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;

QLabelKeys::QLabelKeys(QLabel* parent) : QLabel(parent), qLabel(nullptr)
{
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    windowPixmap = new QPixmap(size());
    x = 50;
    y = 0;
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &QLabelKeys::redraw);
    timer->start(1000/FPS);
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    lastUpdate = 0;
    isPressed[Qt::Key_Up] = false;
    isPressed[Qt::Key_Down] = false;
    isPressed[Qt::Key_Left] = false;
    isPressed[Qt::Key_Right] = false;
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

void QLabelKeys::displayAt(QPixmap *pixmap, int centerx, int centery) {
    int w = pixmap->size().width(), h = pixmap->size().height();
    int top = topBackground + centery - h/2, left = leftBackground + centerx - w/2;
    if(top >= size().height() || top+h <= 0 || left >= size().width() || left+w <= 0)
        return; // pixmap is out of screen
    QPainter painter(windowPixmap);
    painter.drawPixmap(left, top, *pixmap);
}

void QLabelKeys::setCenterBorderLimit(int x, int y) {
    QPainter painter(windowPixmap);
    int winWidth = size().width(), winHeight = size().height();
    int backWidth = backgroundPixmap->size().width(), backHeight = backgroundPixmap->size().height();
    if(backWidth <= winWidth) // Center horizontally
        leftBackground = (winWidth-backWidth)/2;
    else {
        leftBackground = winWidth/2-x;
        leftBackground = max(leftBackground, winWidth - backWidth); // clip at right border
        leftBackground = min(leftBackground, 0); // clip at left border
    }
    if(backHeight <= winHeight)
        topBackground = (winHeight-backHeight)/2;
    else {
        topBackground = winHeight/2-y;
        topBackground = max(topBackground, winHeight - backHeight); // clip at bottom border
        topBackground = min(topBackground, 0); // clip at top border
    }
    painter.drawPixmap(leftBackground, topBackground, *backgroundPixmap);
}

void QLabelKeys::redraw() {
    // Movement
    qint64 now = elapsedTimer->elapsed();
    qint64 elapsed = now - lastUpdate;
    lastUpdate = now;
    if(currentInGameGUI == IN_GAME_GUI_NONE) {
        bool moveVert = isPressed[Qt::Key_Up] != isPressed[Qt::Key_Down];
        bool moveHoriz = isPressed[Qt::Key_Left] != isPressed[Qt::Key_Right];
        int delta;
        if(moveVert && moveHoriz)
            delta = elapsed * MOVEMENT_SPEED_SEC / 1414; // 1000*sqrt(2)
        else
            delta = elapsed * MOVEMENT_SPEED_SEC / 1000;
        if(moveVert) {
            if(isPressed[Qt::Key_Up]) {
                if(y >= delta)
                    y -= delta;
                else
                    y = 0;
            }
            else if(y + delta < backgroundPixmap->size().height())
                y += delta;
            else
                y = backgroundPixmap->size().height()-1;
        }
        if(moveHoriz) {
            if(isPressed[Qt::Key_Left]) {
                if(x >= delta)
                    x -= delta;
                else
                    x = 0;
            }
            else if(x + delta < backgroundPixmap->size().width())
                x += delta;
            else
                x = backgroundPixmap->size().width();
        }
    }

    QPixmap* oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(size());
    setCenterBorderLimit(x, y);
    displayAt(playerPixmap, x, y);
    setPixmap(*windowPixmap);
    delete oldPixmap;
    if(qLabel != nullptr && layout() == nullptr)
    {
        currLayout = new QHBoxLayout;
        currLayout->addWidget(qLabel); // crash here ?
        setLayout(currLayout);
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
        if(!(key->isAutoRepeat())) {
            int keycode = key->key();
            switch(keycode)
            {
                case Qt::Key_Down:
                case Qt::Key_Up:
                case Qt::Key_Left:
                case Qt::Key_Right:
                    isPressed[keycode] = true;
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
                        setLayout(nullptr);
                        delete currLayout;
                        delete qLabel;
                        qLabel = nullptr;
                        //qLabel = new QLabel;
                        //qLabel->clear();
                        //free(qLabel);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        else
            return false;
    }
    else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if(!key->isAutoRepeat()) {
            int keycode = key->key();
            switch(keycode)
            {
                case Qt::Key_Down:
                case Qt::Key_Up:
                case Qt::Key_Left:
                case Qt::Key_Right:
                    isPressed[keycode] = false;
                    break;
                default:
                    return false;
            }
            return true;
        }
        else
            return false;
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
