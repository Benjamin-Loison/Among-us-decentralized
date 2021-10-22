#include "InGameUI.h"
#include "main.h"
#include <QPushButton>
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;
const int X_SPAWN = 5500;
const int Y_SPAWN = 1100;
const QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)},
             colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                             {QColor(20, 156, 20), QColor(8, 99, 64)},
                             {QColor(17, 43, 192), QColor(8, 19, 131)},
                             {QColor(102, 67, 27), QColor(87, 35, 21)},
                             {QColor(193, 17, 17), QColor(120, 8, 57)},
                             {QColor(62, 71, 78), QColor(30, 30, 38)},
                             {QColor(244, 244, 86), QColor(194, 134, 34)}};

InGameUI::InGameUI(QString nickname, QLabel *parent) : QLabel(parent), currPlayer(Player(X_SPAWN, Y_SPAWN, nickname, colors[1][0], colors[1][1])), qLabel(nullptr)
{
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    readyButtonLayout = nullptr;
}

void InGameUI::initialize()
{
    everyoneReady = false;
    otherPlayers.push_back(Player(X_SPAWN, Y_SPAWN, "Test player", colors[0][0], colors[0][1]));
    windowPixmap = new QPixmap(size());
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &InGameUI::redraw);
    timer->start(1000 / FPS);
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    lastUpdate = 0;
    isPressed[Qt::Key_Up] = false;
    isPressed[Qt::Key_Down] = false;
    isPressed[Qt::Key_Left] = false;
    isPressed[Qt::Key_Right] = false;
    initDisplay();
}

bool InGameUI::isCollision(quint16 x, quint16 y)
{
    return collisionImage.pixelColor(x, y) == QColor(255, 0, 0);
}

void InGameUI::initDisplay()
{
    backgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"
    collisionPixmap = getQPixmap("mapCropCollision.png");
    collisionImage = collisionPixmap->toImage();

    if (isCollision(currPlayer.x, currPlayer.y))
    {
        bool found = false;
        for (currPlayer.x = 0; currPlayer.x < backgroundPixmap->size().width() && !found; currPlayer.x++)
            for (currPlayer.y = 0; currPlayer.y < backgroundPixmap->size().height() && !found; currPlayer.y++)
                if (!isCollision(currPlayer.x, currPlayer.y))
                    found = true;
    }
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void InGameUI::displayAt(QPixmap *pixmap, int centerx, int centery, QPainter *painter = nullptr)
{
    int w = pixmap->size().width(), h = pixmap->size().height();
    int top = topBackground + centery - h / 2, left = leftBackground + centerx - w / 2;
    if (top >= size().height() || top + h <= 0 || left >= size().width() || left + w <= 0)
        return; // pixmap is out of screen
    if (painter)
        painter->drawPixmap(left, top, *pixmap);
    else
    {
        QPainter locPainter(windowPixmap);
        locPainter.drawPixmap(left, top, *pixmap);
    }
}

void InGameUI::setCenterBorderLimit(int x, int y, QPainter *painter = nullptr)
{
    bool destroyPainter = painter == nullptr;
    if (destroyPainter)
        painter = new QPainter(windowPixmap);
    int winWidth = size().width(), winHeight = size().height();
    int backWidth = backgroundPixmap->size().width(), backHeight = backgroundPixmap->size().height();
    if (backWidth <= winWidth) // Center horizontally
        leftBackground = (winWidth - backWidth) / 2;
    else
    {
        leftBackground = winWidth / 2 - x;
        leftBackground = max(leftBackground, winWidth - backWidth); // clip at right border
        leftBackground = min(leftBackground, 0);                    // clip at left border
    }
    if (backHeight <= winHeight)
        topBackground = (winHeight - backHeight) / 2;
    else
    {
        topBackground = winHeight / 2 - y;
        topBackground = max(topBackground, winHeight - backHeight); // clip at bottom border
        topBackground = min(topBackground, 0);                      // clip at top border
    }
    painter->drawPixmap(leftBackground, topBackground, *backgroundPixmap);
    if (destroyPainter)
        delete painter;
}

bool InGameUI::performMovement(qint64 elapsed, int dirVert, int dirHoriz)
{
    int delta;
    if (dirVert && dirHoriz)
        delta = elapsed * MOVEMENT_SPEED_SEC / 1414; // 1000*sqrt(2)
    else
        delta = elapsed * MOVEMENT_SPEED_SEC / 1000;
    qint16 nx = currPlayer.x, ny = currPlayer.y;
    if (dirVert == -1)
    {
        if (currPlayer.y >= delta)
            ny = currPlayer.y - delta;
        else if (currPlayer.y > 0)
            currPlayer.y = 0;
        else
            return false;
    }
    else if (dirVert == 1)
    {
        if (currPlayer.y + delta < backgroundPixmap->size().height())
            ny = currPlayer.y + delta;
        else if (currPlayer.y < backgroundPixmap->size().height() - 1)
            ny = backgroundPixmap->size().height() - 1;
        else
            return false;
    }
    if (dirHoriz == -1)
    {
        if (currPlayer.x >= delta)
            nx = currPlayer.x - delta;
        else if (currPlayer.x > 0)
            nx = 0;
        else
            return false;
    }
    else if (dirHoriz == 1)
    {
        if (currPlayer.x + delta < backgroundPixmap->size().width())
            nx = currPlayer.x + delta;
        else if (currPlayer.x < backgroundPixmap->size().width() - 1)
            nx = backgroundPixmap->size().width();
        else
            return false;
    }
    if (!isCollision(nx, ny))
    {
        if (nx < currPlayer.x)
            currPlayer.playerFacingLeft = true;
        else if (nx > currPlayer.x)
            currPlayer.playerFacingLeft = false;
        currPlayer.x = nx;
        currPlayer.y = ny;

        return true;
    }
    else
        return false;
}

void InGameUI::displayPlayer(const Player &player, QPainter *painter = nullptr)
{
    displayAt(player.playerFacingLeft ? player.flippedPixmap : player.playerPixmap, player.x, player.y - player.playerPixmap->size().height() / 2, painter);
    QPainter *newPainter;
    if (painter)
        newPainter = painter;
    else
        newPainter = new QPainter(windowPixmap);
    int fontSizePt = 23;
    newPainter->setFont(QFont("Liberation Sans", fontSizePt));
    QRect textRect(leftBackground + player.x, topBackground + player.y - player.playerPixmap->size().height() - fontSizePt - 5, 1, fontSizePt);
    QRect boundingRect;
    QPen oldPen = newPainter->pen();
    newPainter->setPen(Qt::white);
    newPainter->drawText(textRect, Qt::TextDontClip | Qt::TextSingleLine | Qt::AlignCenter, player.nickname, &boundingRect);
    newPainter->fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
    newPainter->drawText(textRect, Qt::TextDontClip | Qt::TextSingleLine | Qt::AlignCenter, player.nickname, &boundingRect);
    newPainter->setPen(oldPen);
    if (!painter)
        delete newPainter;
}

void InGameUI::redraw()
{
    // Movement
    qint64 now = elapsedTimer->elapsed();
    qint64 elapsed = now - lastUpdate;
    lastUpdate = now;
    if (currentInGameGUI == IN_GAME_GUI_NONE)
    {
        bool moveVert = isPressed[Qt::Key_Up] != isPressed[Qt::Key_Down];
        bool moveHoriz = isPressed[Qt::Key_Left] != isPressed[Qt::Key_Right];
        int dirVert = moveVert ? (isPressed[Qt::Key_Up] ? -1 : 1) : 0;
        int dirHoriz = moveHoriz ? (isPressed[Qt::Key_Left] ? -1 : 1) : 0;
        if (!performMovement(elapsed, dirVert, dirHoriz))
        {
            if (moveVert && moveHoriz)
            {
                if (!performMovement(elapsed, dirVert, 0))
                    performMovement(elapsed, 0, dirHoriz);
            }
            else if (moveVert)
            {
                if (!performMovement(elapsed, dirVert, -1))
                    performMovement(elapsed, dirVert, 1);
            }
            else
            {
                if (!performMovement(elapsed, -1, dirHoriz))
                    performMovement(elapsed, 1, dirHoriz);
            }
        }
    }

    QPixmap *oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(size());
    QPainter painter(windowPixmap);
    setCenterBorderLimit(currPlayer.x, currPlayer.y - currPlayer.playerPixmap->size().height() / 2, &painter);
    //displayAt(playerFacingLeft ? flippedPlayerPixmap : playerPixmap, x, y-playerPixmap->size().height()/2);
    // Display players with ascending y, then ascending x
    QVector<Player *> players;
    players.push_back(&currPlayer);
    for (Player &player : otherPlayers)
        players.push_back(&player);
    qSort(players.begin(), players.end(), [](const Player *a, const Player *b)
          {
              if (a->y != b->y)
                  return a->y < b->y;
              else
                  return a->x < b->x;
          });
    for (Player *player : players)
        displayPlayer(*player, &painter);

    if (!everyoneReady)
    {
        /*
        QRect rect(windowPixmap->size().width() - 100, windowPixmap->size().height() - 50, 100, 50);
        painter.fillRect(rect, QColor(128, 128, 128, 255));
        painter.setFont(QFont("Liberation Sans", 23));
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, "Ready");*/
        if(!readyButtonLayout) {
            qDebug() << "Creating layout";
            readyButtonLayout = new QGridLayout;
            readyButton = new QPushButton("Ready");
            connect(readyButton, &QPushButton::released, this, &InGameUI::onReadyClicked);
            readyButtonLayout->addWidget(readyButton, 0, 0, Qt::AlignBottom | Qt::AlignRight);
            setLayout(readyButtonLayout);
        }
    }
    setPixmap(*windowPixmap);
    delete oldPixmap;
}

void InGameUI::resizeEvent(QResizeEvent *ev)
{
    redraw();
    QLabel::resizeEvent(ev);
}

void InGameUI::onReadyClicked() {
    everyoneReady = true;
    readyButtonLayout->removeWidget(readyButton);
    delete readyButton;
    delete readyButtonLayout;
    readyButtonLayout = nullptr;
    qDebug() << "Ready clicked";
}

bool InGameUI::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (!(key->isAutoRepeat()))
        {
            int keycode = key->key();
            switch (keycode)
            {
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_Left:
            case Qt::Key_Right:
                isPressed[keycode] = true;
                break;
            // https://nerdschalk.com/among-us-keyboard-controls/
            case Qt::Key_E:
                if(everyoneReady) {
                    if (qLabel == nullptr)
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
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (!key->isAutoRepeat())
        {
            int keycode = key->key();
            switch (keycode)
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
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (qLabel != nullptr)
        {
            if (currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
                onMouseEventFixWiring(mouseEvent);
            return true;
        }
    }
    return false;
}
