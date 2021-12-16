#include "InGameUI.h"
#include "main.h"
#include "asteroids.h"
#include "camera.h"
#include <QPushButton>
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;
const int KILL_RANGE_SQUARED = qPow(200, 2);
const int TASK_RANGE_SQUARED = qPow(200, 2);
const int REPORT_RANGE_SQUARED = qPow(200, 2);
const int EMERGENCY_RANGE_SQUARED = qPow(250, 2);
const int KILL_COOLDOWN_SEC = 45;
const int MAX_EMERGENCY_PER_PLAYER = 1;


enum VentsID current_vent = NULL_VENT;

// should make a function to get new player
InGameUI::InGameUI(QLabel* parent) : QLabel(parent), everyoneReady(false), lastUpdate(0), readyButtonLayout(nullptr), currentTask(nullptr), gameMap(nullptr), qLabel(nullptr), lastKillTime(0)
{
    // doing this at the very first window would be nice (when asking nickname etc)
    setWindowIcon(QIcon(assetsFolder + "logo.png"));
    setWindowTitle(tr("Among Us decentralized"));
    // couldn't put all not necessary stuff in initialize not to delay user input ?
}

void InGameUI::initialize(QString nickname)
{
    currPlayer = Player(nickname);
    windowPixmap = new QPixmap(size());
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &InGameUI::redraw);
    timer->start(1000 / FPS);
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    isPressed[Qt::Key_Up] = false;
    isPressed[Qt::Key_Down] = false;
    isPressed[Qt::Key_Left] = false;
    isPressed[Qt::Key_Right] = false;
    initDisplay();
    initDoorsAndRooms();
}

qint64 InGameUI::currTimer() {
    return elapsedTimer->elapsed();
}

bool InGameUI::isCollision(quint16 x, quint16 y)
{
    bool collidesWithDoor = false;
    for(Door &door : doors)
        if(door.collidesWithPosition(x, y)) {
            collidesWithDoor = true;
            break;
        }
    return !currPlayer.isGhost && (collidesWithDoor || collisionImage.pixelColor(x, y) == QColor(255, 0, 0));
}

void InGameUI::initDisplay()
{
    backgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"
    collisionPixmap = getQPixmap("mapCropCollision.png");
    collisionImage = collisionPixmap->toImage(); // what difference between QPixmap and QImage ?
    QPixmap* killButtonPixmap = getQPixmap("killButton.png");
    killButtonImage = killButtonPixmap->toImage();
    QPixmap* reportButtonPixmap = getQPixmap("reportButton.png");
    reportButtonImage = reportButtonPixmap->toImage();
    QPixmap* useButtonPixmap = getQPixmap("useButton.png");
    useButtonImage = useButtonPixmap->toImage();
    QPixmap* playAgainButtonPixmap = getQPixmap("playAgainButton.png");
    playAgainButtonImage = playAgainButtonPixmap->toImage();

    if (isCollision(currPlayer.x, currPlayer.y)) // why is this here ?
    {
        bool found = false;
        for (currPlayer.x = 0; currPlayer.x < backgroundPixmap->size().width() && !found; currPlayer.x++)
            for (currPlayer.y = 0; currPlayer.y < backgroundPixmap->size().height() && !found; currPlayer.y++)
                if (!isCollision(currPlayer.x, currPlayer.y))
                    found = true;
    }
    //setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void InGameUI::initDoorsAndRooms() {
    doors = {
        Door(2307, 824, true), // Upper Engine
        Door(1798, 1525, false),
        Door(3799, 823, true), // Cafeteria
        Door(4718, 2040, false),
        Door(5875, 823, true),
        Door(3199, 1223, false), // MedBay
        Door(2296, 1990, true), // Security
        Door(1798, 2755, false), // Lower Engine
        Door(2307, 3122, true),
        Door(3129, 3556, false), // Electrical
        Door(4001, 3634, true), // Storage
        Door(4718, 2682, false),
        Door(5153, 3220, true)
    };
    rooms = {
        Room(tr("Upper Engine"), 0, QPoint(1900, 1100), {&doors[0], &doors[1]}),
        Room(tr("MedBay"), 1, QPoint(3350, 1800), {&doors[5]}),
        Room(tr("Cafeteria"), 2, QPoint(4850, 1050), {&doors[2], &doors[3], &doors[4]}),
        Room(tr("Weapons"), 3, QPoint(6650, 950), {}),
        Room(tr("Reactor"), 4, QPoint(1150, 2200), {}),
        Room(tr("Security"), 5, QPoint(2600, 2150), {&doors[6]}),
        Room(tr("Admin"), 6, QPoint(5800, 2700), {}),
        Room(tr("O2"), 7, QPoint(6100, 1900), {}),
        Room(tr("Navigation"), 8, QPoint(8050, 2100), {}),
        Room(tr("Lower Engine"), 9, QPoint(1900, 3300), {&doors[7], &doors[8]}),
        Room(tr("Electrical"), 10, QPoint(3500, 3000), {&doors[9]}),
        Room(tr("Storage"), 11, QPoint(4600, 3550), {&doors[10], &doors[11], &doors[12]}),
        Room(tr("Communications"), 12, QPoint(5700, 4050), {}),
        Room(tr("Shields"), 13, QPoint(6650, 3500), {})
    };
}

/**
 * Displays given pixmap centered at map coordinates (centerx, centery).
 * Uses the provided painter if any.
 */
void InGameUI::displayAt(QPixmap* pixmap, int centerx, int centery, QPainter* painter = nullptr)
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

/**
 * Centers the in-game UI at the given coordinates, clipping at borders if necessary.
 * If the map is smaller than the window in some dimension, the map is centered along that dimension.
 * @brief InGameUI::setCenterBorderLimit
 * @param x
 * @param y
 * @param painter A QPainter that will be used for painting the background.
 * If nullptr is passed, a new QPainter will be used instead.
 */
void InGameUI::setCenterBorderLimit(int x, int y, QPainter* painter, QSize s, quint16 offsetX, quint16 offsetY, quint16 sx, quint16 sy)
{
    // should make sure that even if big screen each user see the same map range etc
    if(s == QSize())
    {
        //qInfo() << "s" << s;
        s = size();
    }
    int winWidth = s.width(), winHeight = s.height();
    int backWidth = backgroundPixmap->size().width(), backHeight = backgroundPixmap->size().height();
    if(sx == 0) // a bit overkill to draw everything if most of it is out of the screen
    {
        sx = backWidth;
        sy = backHeight;
    }
    int leftBackgroundTmp, topBackgroundTmp;
    if (backWidth <= winWidth) // Center horizontally
        leftBackgroundTmp = (winWidth - backWidth) / 2;
    else
    {
        //qInfo() << "d" << winWidth << x << backWidth;
        leftBackgroundTmp = winWidth / 2 - x;
        leftBackgroundTmp = max(leftBackgroundTmp, winWidth - backWidth); // clip at right border
        leftBackgroundTmp = min(leftBackgroundTmp, 0);                    // clip at left border
    }
    if (backHeight <= winHeight)
        topBackgroundTmp = (winHeight - backHeight) / 2;
    else
    {
        topBackgroundTmp = winHeight / 2 - y;
        topBackgroundTmp = max(topBackgroundTmp, winHeight - backHeight); // clip at bottom border
        topBackgroundTmp = min(topBackgroundTmp, 0);                      // clip at top border
    }
    //topBackgroundTmp += offsetX;
    //leftBackgroundTmp += offsetY;
    if(sx != backWidth)
    {
        //qInfo() << "b" << offsetX << offsetY << sx << sy;
        //qInfo() << "c" << leftBackgroundTmp << topBackgroundTmp;
        QRectF target(offsetX, offsetY, sx, sy);
        QRectF source(-leftBackgroundTmp, -topBackgroundTmp, /*sx*/winWidth, /*sy*/winHeight);
        painter->drawPixmap(target, *backgroundPixmap, source);
    }
    else if(painter)
        painter->drawPixmap(leftBackgroundTmp, topBackgroundTmp, /*sx, sy,*/ *backgroundPixmap);
    else {
        QPainter locPainter(windowPixmap);
        locPainter.drawPixmap(leftBackgroundTmp, topBackgroundTmp, /*sx, sy,*/ *backgroundPixmap);
    }
    if(sx == backWidth)
    {
        leftBackground = leftBackgroundTmp;
        topBackground = topBackgroundTmp;
    }
}

/**
 * Performs player movement along the specified direction: +/-1 for moving in increasing/decreasing coordinates.
 * @brief InGameUI::performMovement
 * @param elapsed The number of milliseconds that have elapsed since the last frame. Used for calculating the movement distance.
 * @param dirVert Vertical direction. Can be +1 for positive y, 0 for unchanged y or -1 for negative y.
 * @param dirHoriz Horizontal direction. Can be +1 for positive x, 0 for unchanged x or -1 for negative x.
 * @return Whether the movement was successful (i.e. not prevented by map borders or an obstacle).
 */
bool InGameUI::performMovement(qint64 elapsed, int dirVert, int dirHoriz)
{
    if(!dirVert && !dirHoriz) // no movement!
        return true;
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
        currPlayer.moveTo(nx, ny);
        return true;
    }
    else
        return false;
}

QVector<Player *> InGameUI::getOtherPlayersByDistance() {
    QVector<Player *> players;
    for(Player &player : otherPlayers)
        players.push_back(&player);
    int x = currPlayer.x, y = currPlayer.y;
    sort(players.begin(), players.end(), [&/*why ampersand here ?*/](const Player *a, const Player *b) {
        int sqDistA = qPow(a->x - x, 2) + qPow(a->y - y, 2);
        int sqDistB = qPow(b->x - x, 2) + qPow(b->y - y, 2);
        if(sqDistA != sqDistB)
            return sqDistA < sqDistB;
        else if(a->x != b->x)
            return a->x < b->x;
        else if(a->y != b->y)
            return a->y < b->y;
        else
            return a->nickname < b->nickname;
    });
    return players;
}

bool InGameUI::isNearEmergencyButton()
{
    return !currPlayer.isGhost && (distanceToEmergencyButton() < EMERGENCY_RANGE_SQUARED) && currPlayer.numberOfEmergenciesRequested < MAX_EMERGENCY_PER_PLAYER;
}

quint64 InGameUI::distanceToEmergencyButton()
{
    return qPow(currPlayer.x - EMERGENCY_BUTTON_X, 2) + qPow(currPlayer.y - EMERGENCY_BUTTON_Y, 2);
}

bool InGameUI::isThereAnyUsableTaskNear()
{
    return getUsableTasksByDistance().size() > 0;
}

QVector<Task*> InGameUI::getUsableTasksByDistance() {
    QVector<Task*> ret;
    int x = currPlayer.x, y = currPlayer.y;
    for(Task* task : tasks) {
        QPoint pt = task->location;
        if(task->finished)
            continue;
        int dist = qPow(pt.x()-x, 2) + qPow(pt.y()-y, 2);
        if(dist <= TASK_RANGE_SQUARED)
            ret.push_back(task);
    }
    sort(ret.begin(), ret.end(), [&](const Task* task1, const Task* task2) {
        QPoint pt1 = task1->location;
        QPoint pt2 = task2->location;
        int dist1 = qPow(pt1.x() - x, 2) + qPow(pt1.y() - y, 2);
        int dist2 = qPow(pt2.x() - x, 2) + qPow(pt2.y() - y, 2);
        if(dist1 != dist2)
            return dist1 < dist2;
        else if(pt1.x() != pt2.x())
            return pt1.x() < pt2.x();
        return pt1.y() < pt2.y();
    });
    return ret;
}

/**
 * Looks for a corpse to report.
 */
Player* InGameUI::findReportableBody() {
    // Ghost cannot report bodies
    if(currPlayer.isGhost)
        return nullptr;
    int x = currPlayer.x, y = currPlayer.y;
    for(Player* player : getOtherPlayersByDistance()) {
        if(player->showBody) {
            int sqDist = qPow(player->bodyX - x, 2) + qPow(player->bodyY - y, 2);
            if(sqDist <= REPORT_RANGE_SQUARED)
                return player;
        }
    }
    return nullptr;
}

/**
 * Reports a corpse. Does not perform checks.
 */
bool InGameUI::reportBody(Player &p) {
    qDebug() << "Reported player" << p.nickname;
    triggerMeeting(&p);
    return true;
}

/**
 * Looks for a player to kill. The returned Player is in range and alive.
 * Also checks whether the current Player is an alive Impostor.
 */
Player* InGameUI::findKillablePlayer() {
    if(!currPlayer.isImpostor || currPlayer.isGhost)
        return nullptr;
    int x = currPlayer.x, y = currPlayer.y;
    for(Player* player : getOtherPlayersByDistance()) {
        if(!player->isGhost) {
            int sqDist = qPow(player->x - x, 2) + qPow(player->y - y, 2);
            if(sqDist <= KILL_RANGE_SQUARED)
                return player;
            break;
        }
    }
    return nullptr;
}

/**
 * Kills the given Player.
 */
bool InGameUI::killPlayer(Player &p) {
    //if(!currPlayer.isImpostor || p.isGhost) // should be done at a higher level
    //    return false;
    p.isGhost = true;
    p.bodyX = p.x;
    p.bodyY = p.y;
    p.showBody = true;
    qDebug() << "Killed" << p.nickname;
    return true;
}

/**
 * Displays the given Player.
 * @brief InGameUI::displayPlayer
 * @param player
 * @param painter A QPainter that will be used for painting. A fresh one will be used if this is nullptr.
 */
void InGameUI::displayPlayer(const Player &player, QPainter *painter, bool showGhost, quint16 forceX, quint16 forceY)
{
    // Only ghosts see ghosts
    if(showGhost && ((!currPlayer.isGhost && !forceX && !forceY) || !player.isGhost))
    {
        //qInfo("stoped");
        return;
    }
    else if(player.isGhost && (!player.showBody || (forceX && forceY)) && !showGhost)
        return;
    else {
        int x = forceX ? forceX : (player.isGhost && !showGhost) ? player.bodyX : player.x;
        int y = forceY ? forceY : (player.isGhost && !showGhost) ? player.bodyY : player.y;
        QPixmap* toDraw;
        if(!showGhost) {
            if(player.isGhost)
                toDraw = player.deadPixmap;
            else if(player.playerFacingLeft && !forceX && !forceY)
                toDraw = player.flippedPixmap;
            else
                toDraw = player.playerPixmap;
        }
        else {
            if(player.playerFacingLeft && !forceX && !forceY)
                toDraw = player.flippedGhostPixmap;
            else
                toDraw = player.ghostPixmap;
        }
        //qInfo() << player.isGhost << y - toDraw->size().height() / 2;
        displayAt(toDraw, x, y - toDraw->size().height() / 2, painter);
        QPainter* newPainter = painter ? painter : new QPainter(windowPixmap);
        int fontSizePt = 23;
        newPainter->setFont(QFont("Liberation Sans", fontSizePt));
        QRect textRect(leftBackground + x, topBackground + y - toDraw->size().height() - fontSizePt - 5, 1, fontSizePt);
        QRect boundingRect;
        QPen oldPen = newPainter->pen();
        newPainter->setPen(Qt::white);
        newPainter->drawText(textRect, Qt::TextDontClip | Qt::AlignCenter, player.nickname, &boundingRect);
        newPainter->fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
        newPainter->drawText(textRect, Qt::TextDontClip | Qt::AlignCenter, player.nickname, &boundingRect);
        newPainter->setPen(oldPen);
        if(!painter)
            delete newPainter;
    }
}

bool InGameUI::isWinScreen()
{
    return currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES || currentInGameGUI == IN_GAME_GUI_WIN_IMPOSTORS;
}

void InGameUI::displayDoors(QPainter* painter)
{
    for(Door& door : doors)
        door.draw(painter, leftBackground, topBackground);
}

void InGameUI::displayPlayers(QPainter* painter)
{
    // Display players with ascending y, then ascending x.
    // Display ghosts above alive players and dead bodies.
    QVector<Player*> players;
    players.push_back(&currPlayer);
    for (Player &player : otherPlayers)
        players.push_back(&player);
    sort(players.begin(), players.end(), [](const Player *a, const Player *b)
          {
              int aY = a->isGhost ? a->bodyY : a->y,
                  aX = a->isGhost ? a->bodyX : a->x,
                  bY = b->isGhost ? b->bodyY : b->y,
                  bX = b->isGhost ? b->bodyX : b->x;
              if(aY != bY)
                  return aY < bY;
              else if(aX != bX)
                  return aX < bX;
              return a->nickname.compare(b->nickname) < 0;
          });
    for (Player* player : players)
        displayPlayer(*player, painter, false);
    sort(players.begin(), players.end(), [](const Player *a, const Player *b)
          {
              if (a->y != b->y)
                  return a->y < b->y;
              else if(a->x != b->x)
                  return a->x < b->x;
              return a->nickname.compare(b->nickname) < 0;
          });
    for (Player* player : players)
        displayPlayer(*player, painter, true);
}

/**
 * Performs player movement, then redraws the in-game UI. Meant to be called at each frame.
 * @brief InGameUI::redraw
 */
void InGameUI::redraw()
{
    qint64 now = elapsedTimer->elapsed();
    qint64 elapsed = now - lastUpdate;
    lastUpdate = now;
    QSize qSize = size();
    int qWidth = qSize.width(),
        qHeight = qSize.height();
    if(isWinScreen())
    {
        leftBackground = 0;
        topBackground = 0;
        QPixmap *oldPixmap = windowPixmap;
        windowPixmap = new QPixmap(qSize);
        QPainter painter(windowPixmap);
        painter.fillRect(0, 0, qWidth, qHeight, Qt::black);
        QString winningTeam = currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES ? tr("crewmates") : tr("impostors");
        painter.setPen(currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES ? Qt::blue : Qt::red);
        QString title = firstUppercase(QString(tr("%1' victory")).arg(winningTeam));
        painter.setFont(QFont("arial", 25));
        quint16 middleX = qWidth / 2, middleY = qHeight / 2;
        drawCenteredText(&painter, middleX, qHeight / 10, title);

        QList<Player*> players;
        if((currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES) == (!currPlayer.isImpostor))
            players.push_back(&currPlayer);
        QList<QString> keys = otherPlayers.keys();
        quint8 otherPlayersSize = keys.size();
        for(quint8 otherPlayersIndex = 0; otherPlayersIndex < otherPlayersSize; otherPlayersIndex++)
        {
            QString key = keys[otherPlayersIndex];
            Player* player = &otherPlayers[key];
            if((currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES) == (!player->isImpostor))
                players.push_back(player);
        }

        quint8 playersSize = players.size();
        for(quint8 playersIndex = 0; playersIndex < playersSize; playersIndex++)
        {
            Player* player = players[playersIndex];
            //qInfo() << "winner:" << player->nickname << middleX << middleY << qSize.width() << qSize.height();
            for(bool showGhost : {false, true})
                displayPlayer(*player, &painter, showGhost, (quint16)((2*middleX + player->playerPixmap->width() * (2*playersIndex - playersSize + 1))/2), middleY + player->playerPixmap->height()/2);
        }
        painter.drawImage(qWidth - 130, qHeight - 130, playAgainButtonImage);

        setPixmap(*windowPixmap);
        delete oldPixmap;
        return;
    }
    // Movement
    if (currentInGameGUI == IN_GAME_GUI_NONE || currentInGameGUI == IN_GAME_GUI_MAP)
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

    // Asteroids
    if(currentInGameGUI == IN_GAME_GUI_ASTEROIDS)
        redrawAsteroids(now);
    else if(currentInGameGUI == IN_GAME_GUI_CAMERA)
        redrawCamera();

    QPixmap* oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(qSize);
    QPainter painter(windowPixmap);
    setCenterBorderLimit(currPlayer.x, currPlayer.y - currPlayer.playerPixmap->size().height() / 2, &painter);
    // there is a misorder with door/player that can't be solved by just moving this for loop
    // Display doors above players (could largely be improved!)
    displayDoors(&painter);

    displayPlayers(&painter);

    int fontSizePt = 23;
    // Impostor message
    if(currPlayer.isImpostor) {
        painter.setFont(QFont("Liberation Sans", fontSizePt));
        QRect textRect(0, 0, 1, fontSizePt);
        QRect boundingRect;
        //QPen oldPen = painter.pen();
        painter.setPen(Qt::red);
        QString text = tr("You are an Impostor !");
        painter.drawText(textRect, Qt::TextDontClip | Qt::AlignLeft, text, &boundingRect); // why twice, is it a kind of shade ?
        painter.fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
        painter.drawText(textRect, Qt::TextDontClip | Qt::AlignLeft, text);
        //painter.setPen(oldPen);
    }

    // For debugging purposes: show current location - should make a boolean to choose whether or not to display location
    /*QRect textRect(qWidth - 1, 0, 1, fontSizePt);
    QRect boundingRect;
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y), &boundingRect);
    boundingRect.setLeft(qWidth - 1 - boundingRect.width());
    boundingRect.setRight(qWidth - 1);
    painter.fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
    painter.drawText(boundingRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y));
    */
    // Game buttons
    if(everyoneReady)
    {
        // could choose not to display use, report, kill buttons on meeting because not useable and especially killing button because the counter is decreasing
        if(findKillablePlayer())
        {
            qint64 currentTime = QDateTime::currentSecsSinceEpoch();
            if(currentTime >= lastKillTime + KILL_COOLDOWN_SEC)
                painter.drawImage(qWidth - 220, qHeight - 110, killButtonImage);
            else
            {
                QImage killButtonTemporaryImage = killButtonImage; // a gray version would be nice
                QPainter* killButtonPainter = new QPainter(&killButtonTemporaryImage);
                QFont font = QFont("arial", 40, QFont::ExtraBold);
                killButtonPainter->setFont(font);
                killButtonPainter->setPen(Qt::white);
                drawCenteredText(killButtonPainter, 55, 55, QString::number(KILL_COOLDOWN_SEC - (currentTime - lastKillTime)));
                painter.drawImage(qWidth - 220, qHeight - 110, killButtonTemporaryImage);
                delete(killButtonPainter);
            }
        }
        if(findReportableBody())
            painter.drawImage(qWidth - 110, qHeight - 110, reportButtonImage);
        if(isThereAnyUsableTaskNear() || isNearEmergencyButton() || isNearCamera() || (isThereAnyVentNear(QPoint(currPlayer.x, currPlayer.y)) && (current_vent== NULL_VENT)))
            painter.drawImage(qWidth - 110, qHeight - 220, useButtonImage);
    }

    // Ready button
    if(!everyoneReady && !readyButtonLayout)
    {
        readyButtonLayout = new QGridLayout;
        readyButton = new QPushButton(tr("Ready"));
        readyButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        connect(readyButton, &QPushButton::released, this, &InGameUI::onReadyClicked);
        readyButtonLayout->addWidget(readyButton, 0, 0, Qt::AlignBottom | Qt::AlignRight);
        setLayout(readyButtonLayout);
    }

    // Game map
    if(gameMap)
        gameMap->redraw();

    setPixmap(*windowPixmap);
    delete oldPixmap;
}

void InGameUI::resizeEvent(QResizeEvent *ev)
{
    redraw();
    QLabel::resizeEvent(ev);
}

void InGameUI::onReadyClicked() {
    if(!currPlayer.isReady)
    {
        if(getPlayersNumber() >= MINIMAL_NUMBER_OF_PLAYERS)
        {
            qDebug() << "Ready clicked";
            currPlayer.isReady = true;
            readyButton->setText(tr("Waiting for other players"));
            readyButton->setEnabled(false);
            sendToAll("ready");
            checkEverybodyReady();
        }
        else
            qDebug() << "Still waiting for players..."; // could display it on the button by default how many we are waiting
    }
}

void InGameUI::setImpostor(QString nickname)
{
    Player* player = getPlayer(nickname);
    player->isImpostor = true;
}

void InGameUI::onEverybodyReadySub(bool threadSafe)
{
    //if(!threadSafe && waitingAnswersNumber > 0) return;
    /*while(waitingAnswersNumber > 0) // should do this but it's not working... the problem is that we have to wait network but we are in the network thread
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }*/
    /*sendToAll("Random " + privateRandom); /// TODO check the privateRandom at the end of the game
    waitingAnswersNumber = otherPlayers.size();
    while(waitingAnswersNumber > 0)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }*/
    tasks = getTasksAsPointers(getRandomTasks(privateRandom)); // should be salted with common random

    if(isFirstToRun) // temporary
    {
        // TODO: use IMPOSTOR_NUMBER
        quint8 imposterPlayerIndex = QRandomGenerator::global()->bounded(getPlayersNumber());
        QString nickname = currPlayer.nickname;
        QList<QString> peerAddresses = otherPlayers.keys();
        if(imposterPlayerIndex > 0)
            nickname = otherPlayers[peerAddresses[imposterPlayerIndex - 1]].nickname;
        setImpostor(nickname);
        sendToAll("Imposter " + nickname);
    }
    readyButtonLayout->removeWidget(readyButton);
    delete readyButton;
    delete readyButtonLayout;
    readyButtonLayout = nullptr;

    resetAllPlayers();

    lastKillTime = QDateTime::currentSecsSinceEpoch();
}

QList<Player*> InGameUI::getAllPlayers()
{
    QList<Player*> players;
    players.push_back(&currPlayer);
    for(Player& player : otherPlayers)
        players.push_back(&player);
    return players;
}

void InGameUI::teleportAllPlayers()
{
    QList<QString> peerAddresses = otherPlayers.keys();
    for(QString peerAddress : peerAddresses)
        movePlayer(peerAddress, X_SPAWN, Y_SPAWN, true);
    currPlayer.x = X_SPAWN;
    currPlayer.y = Y_SPAWN;
    currPlayer.playerFacingLeft = false;
}

void InGameUI::resetAllPlayers()
{
    // could use getAllPlayers function
    // teleport all players and hide bodies
    QList<QString> peerAddresses = otherPlayers.keys();
    for(QString peerAddress : peerAddresses)
        hidePlayerBodyIfDead(peerAddress);
    teleportAllPlayers();
    if(currPlayer.isGhost) currPlayer.showBody = false;
}

void InGameUI::onEverybodyReady(bool threadSafe)
{
    everyoneReady = true;
    privateRandom = randomHex(512);
    QString randomHashed = SHA512(privateRandom);
    //sendToAll("RandomHashed " + randomHashed);
    waitingAnswersNumber = otherPlayers.size();
    quint8 crewmates = waitingAnswersNumber + 1 - IMPOSTOR_NUMBER;
    gameCommonTasks = crewmates * commonTasks;
    gameLongTasks = crewmates * longTasks;
    gameShortTasks = crewmates * shortTasks;
    //if(threadSafe)
        onEverybodyReadySub(threadSafe);
    //else
    //    needEverybodyReadyCall = true;
}

bool isAliveCrewmate(Player* player)
{
    return !player->isGhost && !player->isImpostor;
}

quint8 InGameUI::getAlivePlayersNumber()
{
    // could use getAllPlayers maybe other functions too need this function
    quint8 alivePlayersNumber = currPlayer.isGhost ? 0 : 1;
    for(Player& player : otherPlayers)
        if(!player.isGhost)
            alivePlayersNumber++;
    return alivePlayersNumber;
}

// see voted for the moment would be nice
void InGameUI::executeVote(QString voteStr, Player *voter)
{
    meetingWidget->waitingVotes--;
    if(voteStr != "Skip")
    {
        QString voteNickname = voteStr.replace("Voted ", "");
        meetingWidget->votes[voteNickname]++;
        meetingWidget->votesByPlayer[voter->nickname] = getPlayer(voteNickname);
    }
    else {
        meetingWidget->skipVotes++;
        meetingWidget->votesByPlayer[voter->nickname] = nullptr;
    }
    if(meetingWidget->waitingVotes == 0)
    {
        meetingResultsWidget = new MeetingResultsUI(this, meetingWidget->votes, meetingWidget->votesByPlayer, meetingWidget->skipVotes);
        closeMeetingUI();
        currHLayout = makeCenteredLayout(meetingResultsWidget);
        setLayout(currHLayout);
        currentInGameGUI = IN_GAME_GUI_MEETING;
    }
}

void InGameUI::onAllProceeded(Player* whoToKill) {
    if(whoToKill)
        killPlayer(*whoToKill);
    resetAllPlayers(); // could also TP before opening meeting interface
    lastKillTime = QDateTime::currentSecsSinceEpoch();
    delete meetingResultsWidget;
    delete currHLayout;
    meetingResultsWidget = nullptr;
    currHLayout = nullptr;
    currentInGameGUI = IN_GAME_GUI_NONE;
    checkEndOfTheGame();
}

quint8 InGameUI::getAliveCrewmatesNumber()
{
    QList<QString> keys = otherPlayers.keys();
    quint8 aliveCrewmates = isAliveCrewmate(&currPlayer) ? 1 : 0, keysSize = keys.size();
    for(quint8 keysIndex = 0; keysIndex < keysSize; keysIndex++)
    {
        QString key = keys[keysIndex];
        Player* player = &otherPlayers[key];
        if(isAliveCrewmate(player))
            aliveCrewmates++;
    }
    return aliveCrewmates;
}

quint8 InGameUI::getAliveImpostorsNumber()
{
    return getAlivePlayersNumber() - getAliveCrewmatesNumber();
}

void InGameUI::unreadyTeleportEveryone() {
    for(Player* player : getAllPlayers())
        player->isReady = false;
    teleportAllPlayers();
    everyoneReady = false;
}

void InGameUI::checkEndOfTheGame() // could optimize by precising which checkEndOfTheGame (task or death) to optimize
{
    if(gameCommonTasks + gameLongTasks + gameShortTasks == 0)
    {
        qInfo("all crewmates tasks done !");
        unreadyTeleportEveryone();
        currentInGameGUI = IN_GAME_GUI_WIN_CREWMATES;
    }
    else
    {
        if(getAliveCrewmatesNumber() == 0)
        {
            qInfo("impostors win !");
            unreadyTeleportEveryone();
            currentInGameGUI = IN_GAME_GUI_WIN_IMPOSTORS;
        }
        else if(getAliveImpostorsNumber() == 0)
        {
            qInfo("crewmates win !");
            unreadyTeleportEveryone();
            currentInGameGUI = IN_GAME_GUI_WIN_CREWMATES;
        }
    }
}

void InGameUI::taskFinished(TaskTime taskTime)
{
    switch(taskTime)
    {
        case TASK_COMMON:
            inGameUI->gameCommonTasks--;
            break;
        case TASK_LONG:
            inGameUI->gameLongTasks--;
            break;
        default:
            inGameUI->gameShortTasks--;
    }
}

void InGameUI::finishTask() {
    if(!currentTask)
        return;
    currentTask->finished = true;
    TaskTime taskTime = taskTimes[currentTask->taskType];
    taskFinished(taskTime);
    sendToAll("finished " + taskTimeToString(taskTime));
}

void InGameUI::closeTask() {
    if(!currentTask)
        return;
    switch(currentInGameGUI) {
        case IN_GAME_GUI_FIX_WIRING:
        onCloseFixWiring();
        break;
        case IN_GAME_GUI_ASTEROIDS:
        onCloseAsteroids();
        break;
        case IN_GAME_GUI_ENTER_ID_CODE :
        onCloseEnterIDCode();
        break;
        case IN_GAME_GUI_ALIGN_ENGINE:
        onCloseAlignEngine();
        break;
        default:;
    }
    currentTask = nullptr;
    currentInGameGUI = IN_GAME_GUI_NONE;
    delete currHLayout;
    delete qLabel;
    qLabel = nullptr;
}

void InGameUI::onClickUse() {
    if(isNearEmergencyButton())
    {
        triggerMeeting();
        currPlayer.numberOfEmergenciesRequested++;
        return;
    }

    if((isThereAnyVentNear(QPoint(currPlayer.x, currPlayer.y)))&& (current_vent==NULL_VENT)){
        current_vent = VentNear(QPoint(currPlayer.x, currPlayer.y));
        QPoint new_pos = PosOfVent(current_vent);
        currPlayer.x = new_pos.x();
        currPlayer.y = new_pos.y();
        currentInGameGUI = IN_GAME_GUI_VENT;
        qLabel = EnterVent(current_vent);
        currHLayout = new QHBoxLayout;
        currHLayout->addStretch();
        currHLayout->addWidget(qLabel);
        currHLayout->addStretch();
        setLayout(currHLayout);
        return;
    }

    QVector<Task*> usableTasks = getUsableTasksByDistance();
    if(usableTasks.size() > 0) {
        Task* task = usableTasks[0];
        switch(task->taskType) {
        case TASK_FIX_WIRING:
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_FIX_WIRING;
            qLabel = getFixWiring();
            break;
        case TASK_ASTEROIDS:
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_ASTEROIDS;
            qLabel = getAsteroids(elapsedTimer->elapsed());
            break;
        case TASK_ENTER_ID_CODE :
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_ENTER_ID_CODE;
            qLabel = getEnterIDCode();
            break;
        case TASK_ALIGN_ENGINE :
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_ALIGN_ENGINE;
            qLabel = getAlignEngine();
            break;
        default:
            return;
        }
        currHLayout = new QHBoxLayout;
        currHLayout->addStretch();
        currHLayout->addWidget(qLabel);
        currHLayout->addStretch();
        setLayout(currHLayout);
    }
    else if(isNearCamera()) // could check what minimal distance in order to know whether the player want task or camera, but it should be another icon btw
    {
        currentInGameGUI = IN_GAME_GUI_CAMERA;
        qLabel = getCamera();
        currHLayout = new QHBoxLayout;
        currHLayout->addStretch();
        currHLayout->addWidget(qLabel);
        currHLayout->addStretch();
        setLayout(currHLayout);
    }
}

void InGameUI::onClickReport() {
    Player* reportable = findReportableBody();
    if(reportable)
        reportBody(*reportable);
}

void InGameUI::onClickKill() {
    // Game logic verifications are performed in findKillablePlayer()
    if(QDateTime::currentSecsSinceEpoch() >= lastKillTime + KILL_COOLDOWN_SEC)
    {
        Player* killable = findKillablePlayer();
        if(killable)
        {
            killPlayer(*killable);
            lastKillTime = QDateTime::currentSecsSinceEpoch();
            inGameUI->checkEndOfTheGame();
            sendToAll("Kill " + killable->nickname);
        }
    }
}

void InGameUI::openMap() {
    if(gameMap || currentInGameGUI != IN_GAME_GUI_NONE || !everyoneReady)
        return;
    gameMap = new GameMap(this);
    currHLayout = makeCenteredLayout(gameMap);
    setLayout(currHLayout);
    currentInGameGUI = IN_GAME_GUI_MAP;
}

void InGameUI::closeMap() {
    if(!gameMap)
        return;
    delete gameMap;
    delete currHLayout;
    gameMap = nullptr;
    currHLayout = nullptr;
    currentInGameGUI = IN_GAME_GUI_NONE;
}

void InGameUI::triggerMeeting(Player* reportedPlayer) {
    if(reportedPlayer)
        sendToAll("Report " + reportedPlayer->nickname);
    else
        sendToAll("Emergency meeting");
    openMeetingUI(reportedPlayer, &currPlayer);
}

/**
 * Opens meeting UI either when a corresponding packet is received from
 * some other client, or the current player reports a player.
 * If the meeting is triggered by a dead body report, the corresponding
 * body is made invisible.
 */
void InGameUI::openMeetingUI(Player* reportedPlayer, Player* reportingPlayer) {
    if(currentTask)
        closeTask();
    if(currentInGameGUI == IN_GAME_GUI_MAP)
        closeMap();
    if(currentInGameGUI == IN_GAME_GUI_VENT){
        ExitVent();
        current_vent = NULL_VENT;
        currentInGameGUI = IN_GAME_GUI_NONE;
        delete currHLayout;
        delete qLabel;
        qLabel = nullptr;
    }
    meetingWidget = new MeetingUI(this, reportedPlayer, reportingPlayer);

    currHLayout = makeCenteredLayout(meetingWidget);
    setLayout(currHLayout);
    currentInGameGUI = IN_GAME_GUI_MEETING;
}

void InGameUI::closeMeetingUI() {
    if(!meetingWidget)
        return;
    delete meetingWidget;
    delete currHLayout;
    meetingWidget = nullptr;
    currHLayout = nullptr;
    currentInGameGUI = IN_GAME_GUI_NONE;
}

void InGameUI::keyPressEvent(QKeyEvent *key) {
    if (key->isAutoRepeat())
        return;
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
            if (qLabel == nullptr && currentInGameGUI == IN_GAME_GUI_NONE)
                onClickUse();
            else if(qLabel){
                if (currentInGameGUI == IN_GAME_GUI_VENT){    
                ExitVent();
                current_vent = NULL_VENT;
                currentInGameGUI = IN_GAME_GUI_NONE;
                delete currHLayout;
                delete qLabel;
                qLabel = nullptr;
                }

                else {closeTask();};
            }
                
        }
        break;
    case Qt::Key_K:
        if(everyoneReady && currentInGameGUI == IN_GAME_GUI_NONE)
            onClickKill();
        break;
    case Qt::Key_R:
        if(everyoneReady && currentInGameGUI == IN_GAME_GUI_NONE)
            onClickReport();
        break;
    case Qt::Key_M:
        if(currentInGameGUI == IN_GAME_GUI_NONE)
            openMap();
        else if(gameMap)
            closeMap();
        break;
    default:;
    }
    QLabel::keyPressEvent(key);
}

void InGameUI::keyReleaseEvent(QKeyEvent* key) {
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
        default:;
        }
    }
    QLabel::keyReleaseEvent(key);
}

void InGameUI::mouseMoveEvent(QMouseEvent *mouseEvent) {
    if (qLabel && mouseEvent->buttons() & Qt::LeftButton)
    {
        if (currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
            onMouseEventFixWiring(mouseEvent);
        else if (currentInGameGUI == IN_GAME_GUI_ALIGN_ENGINE)
            onMouseEventAlignEngine(mouseEvent);
    }
    QLabel::mouseMoveEvent(mouseEvent);
}

void InGameUI::mousePressOrDoubleClick(QMouseEvent *mouseEvent) {
    if(mouseEvent->button() == Qt::LeftButton) {
        int mouseX = mouseEvent->x(), mouseY = mouseEvent->y();
        int width = size().width(), height = size().height();
        bool isBottomRight = mouseX >= width-110 && mouseX < width && mouseY >= height-110 && mouseY < height;
        if(everyoneReady) {
            if(currentInGameGUI == IN_GAME_GUI_NONE) {
                if(mouseX >= width-220 && mouseX < width-110 && mouseY >= height-110 && mouseY < height && findKillablePlayer())
                    onClickKill();
                else if(isBottomRight && findReportableBody())
                    onClickReport();
                else if(mouseX >= width-110 && mouseX < width && mouseY >= height-220 && mouseY < height-110 && (isThereAnyUsableTaskNear() || isNearEmergencyButton() || isThereAnyVentNear(QPoint(currPlayer.x, currPlayer.y)) || isNearCamera()))
                    onClickUse();
            }
            else if(currentInGameGUI == IN_GAME_GUI_MAP)
                gameMap->onLeftOrDoubleClick(mouseEvent);
            else if(currentInGameGUI == IN_GAME_GUI_ASTEROIDS) {
                onMouseEventAsteroids(mouseEvent);
            }
            else if (currentInGameGUI == IN_GAME_GUI_ENTER_ID_CODE)
                onMouseEventEnterIDCode(mouseEvent);
            else if (currentInGameGUI == IN_GAME_GUI_ALIGN_ENGINE)
                onMouseEventAlignEngine(mouseEvent);
            else if (currentInGameGUI = IN_GAME_GUI_VENT){
                VentsID new_vent = onMouseEventVent(current_vent ,mouseEvent);
                if (new_vent!=NULL_VENT){
                    ExitVent();
                    current_vent = new_vent;
                    currentInGameGUI = IN_GAME_GUI_NONE;
                    delete currHLayout;
                    delete qLabel;
                    qLabel = nullptr;
                    QPoint new_pos = PosOfVent(new_vent);
                    currPlayer.x = new_pos.x();
                    currPlayer.y = new_pos.y();
                    currentInGameGUI = IN_GAME_GUI_VENT;
                    qLabel = EnterVent(new_vent);
                    currHLayout = new QHBoxLayout;
                    currHLayout->addStretch();
                    currHLayout->addWidget(qLabel);
                    currHLayout->addStretch();
                    setLayout(currHLayout);

                }
            }
        }
        else if(isWinScreen())
        {
            if(isBottomRight)
            {
                // could almost bypass waiting first ready phase - I thought to do a ready like on the win screen before the real ready but clicking on play again button to switch to first ready interface is better
                for(Player* player : getAllPlayers())
                {
                    player->isImpostor = false;
                    player->isGhost = false;
                    player->showBody = false;
                    player->numberOfEmergenciesRequested = 0;
                }
                currentInGameGUI = IN_GAME_GUI_NONE;
            }
        }
    }
}

void InGameUI::mousePressEvent(QMouseEvent *mouseEvent) {
    mousePressOrDoubleClick(mouseEvent);
    QLabel::mousePressEvent(mouseEvent);
}

void InGameUI::mouseDoubleClickEvent(QMouseEvent *mouseEvent) {
    mousePressOrDoubleClick(mouseEvent);
    QLabel::mouseDoubleClickEvent(mouseEvent);
}

quint8 InGameUI::getPlayersNumber()
{
    return otherPlayers.size() + (currPlayer.nickname == "" ? 0 : 1);
}

void InGameUI::spawnOtherPlayer(QString peerAddress, QString otherPlayerNickname)
{
    otherPlayers[peerAddress] = Player(otherPlayerNickname);
}

void InGameUI::setFacingLeftPlayer(QString peerAddress)
{
    otherPlayers[peerAddress].playerFacingLeft = true;
}

void InGameUI::movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp)
{
    Player* player = &otherPlayers[peerAddress];
    if(tp)
        player->playerFacingLeft = false;
    else if(x != player->x) // otherwise if just change vertically not logical
        player->playerFacingLeft = x < player->x;
    player->x = x;
    player->y = y;
}

void InGameUI::hidePlayerBodyIfDead(QString peerAddress)
{
    Player* player = &otherPlayers[peerAddress];
    if(player->isGhost) player->showBody = false; // could almost make a function also to treat currPlayer
}

void InGameUI::checkEverybodyReady(bool threadSafe)
{
    if(!currPlayer.isReady) return;
    if(all_of(otherPlayers.begin(), otherPlayers.end(), [](Player player){return player.isReady;}))
        onEverybodyReady(threadSafe);
    /*QList<QString> peerAddresses = otherPlayers.keys();
    for(QString peerAddress : peerAddresses)
    {
        if(!otherPlayers[peerAddress].isReady)
        {
            //qInfo() << peerAddress << peerAddressesSize;
            return;
        }
    }*/
}

void InGameUI::setPlayerReady(QString peerAddress, bool threadSafe)
{
    Player* player = &otherPlayers[peerAddress];
    player->isReady = true;
    //if(!threadSafe)
        checkEverybodyReady(/*threadSafe*/);
}

QPixmap* InGameUI::getBackgroundPixmap() {
    return backgroundPixmap;
}

QVector<Task*> InGameUI::getTasks() {
    return tasks;
}

Player* InGameUI::getPlayer(QString nickname)
{
    if(currPlayer.nickname == nickname) return &currPlayer;
    QList<QString> peerAddresses = otherPlayers.keys();
    for(QString peerAddress : peerAddresses)
    {
        Player* player = &otherPlayers[peerAddress];
        if(player->nickname == nickname)
            return player;
    }
    return nullptr;
}
