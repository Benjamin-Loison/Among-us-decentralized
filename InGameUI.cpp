#include "InGameUI.h"
#include "main.h"
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;

InGameUI::InGameUI(QLabel* parent) : QLabel(parent), qLabel(nullptr)
{
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    windowPixmap = new QPixmap(size());
    x = 50;
    y = 0;
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &InGameUI::redraw);
    timer->start(1000/FPS);
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    lastUpdate = 0;
    isPressed[Qt::Key_Up] = false;
    isPressed[Qt::Key_Down] = false;
    isPressed[Qt::Key_Left] = false;
    isPressed[Qt::Key_Right] = false;
    playerFacingLeft = false;
    display();
}

bool InGameUI::isCollision(quint16 x, quint16 y) {
    return collisionImage.pixelColor(x, y) == QColor(255, 0, 0);
}

void InGameUI::display()
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
    collisionPixmap = getQPixmap("mapCropCollision.png");
    collisionImage = collisionPixmap->toImage();

    if(isCollision(x,y)) {
        bool found = false;
        for(x = 0; x < backgroundPixmap->size().width() && !found; x++)
            for(y = 0; y < backgroundPixmap->size().height() && !found; y++)
                if(!isCollision(x,y))
                    found = true;

    }

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
    flippedPlayerPixmap = new QPixmap(playerPixmap->transformed(QTransform().scale(-1,1)));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void InGameUI::displayAt(QPixmap *pixmap, int centerx, int centery) {
    int w = pixmap->size().width(), h = pixmap->size().height();
    int top = topBackground + centery - h/2, left = leftBackground + centerx - w/2;
    if(top >= size().height() || top+h <= 0 || left >= size().width() || left+w <= 0)
        return; // pixmap is out of screen
    QPainter painter(windowPixmap);
    painter.drawPixmap(left, top, *pixmap);
}

void InGameUI::setCenterBorderLimit(int x, int y) {
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

bool InGameUI::performMovement(qint64 elapsed, int dirVert, int dirHoriz) {
    int delta;
    if(dirVert && dirHoriz)
        delta = elapsed * MOVEMENT_SPEED_SEC / 1414; // 1000*sqrt(2)
    else
        delta = elapsed * MOVEMENT_SPEED_SEC / 1000;
    qint16 nx = x, ny = y;
    if(dirVert == -1) {
            if(y >= delta)
                ny = y - delta;
            else if(y > 0)
                y = 0;
            else
                return false;
    }
    else if(dirVert == 1) {
        if(y + delta < backgroundPixmap->size().height())
            ny = y + delta;
        else if(y < backgroundPixmap->size().height()-1)
            ny = backgroundPixmap->size().height()-1;
        else
            return false;
    }
    if(dirHoriz == -1) {
        if(x >= delta)
            nx = x - delta;
        else if(x > 0)
            nx = 0;
        else
            return false;
    }
    else if(dirHoriz == 1) {
        if(x + delta < backgroundPixmap->size().width())
            nx = x + delta;
        else if(x < backgroundPixmap->size().width()-1)
            nx = backgroundPixmap->size().width();
        else
            return false;
    }
    if(!isCollision(nx, ny)) {
        if(nx < x)
            playerFacingLeft = true;
        else if(nx > x)
            playerFacingLeft = false;
        x = nx;
        y = ny;

        return true;
    }
    else
        return false;
}

void InGameUI::redraw() {
    // Movement
    qint64 now = elapsedTimer->elapsed();
    qint64 elapsed = now - lastUpdate;
    lastUpdate = now;
    if(currentInGameGUI == IN_GAME_GUI_NONE) {
        bool moveVert = isPressed[Qt::Key_Up] != isPressed[Qt::Key_Down];
        bool moveHoriz = isPressed[Qt::Key_Left] != isPressed[Qt::Key_Right];
        int dirVert = moveVert ? (isPressed[Qt::Key_Up] ? -1 : 1) : 0;
        int dirHoriz = moveHoriz ? (isPressed[Qt::Key_Left] ? -1 : 1) : 0;
        if(!performMovement(elapsed, dirVert, dirHoriz)) {
            if(moveVert && moveHoriz) {
                if(!performMovement(elapsed, dirVert, 0))
                    performMovement(elapsed, 0, dirHoriz);
            }
            else if(moveVert) {
                if(!performMovement(elapsed, dirVert, -1))
                    performMovement(elapsed, dirVert, 1);
            }
            else {
                if(!performMovement(elapsed, -1, dirHoriz))
                    performMovement(elapsed, 1, dirHoriz);
            }
        }
    }

    QPixmap* oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(size());
    setCenterBorderLimit(x, y-playerPixmap->size().height()/2);
    displayAt(playerFacingLeft ? flippedPlayerPixmap : playerPixmap, x, y-playerPixmap->size().height()/2);
    setPixmap(*windowPixmap);
    delete oldPixmap;
}

void InGameUI::resizeEvent(QResizeEvent* ev) {
    redraw();
    QLabel::resizeEvent(ev);
}

bool InGameUI::eventFilter(QObject* obj, QEvent* event)
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
                        currLayout = new QHBoxLayout;
                        currLayout->addWidget(qLabel);
                        setLayout(currLayout);
                    }
                    else
                    {
                        currentInGameGUI = IN_GAME_GUI_NONE;
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
