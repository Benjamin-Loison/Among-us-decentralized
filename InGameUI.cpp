#include "InGameUI.h"
#include "main.h"
#include "asteroids.h"
#include <QPushButton>
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;
const int KILL_RANGE_SQUARED = qPow(200, 2);
const int TASK_RANGE_SQUARED = qPow(200, 2);
const int REPORT_RANGE_SQUARED = qPow(200, 2);
const int EMERGENCY_RANGE_SQUARED = qPow(250, 2);
const QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)},
             colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                             {QColor(20, 156, 20), QColor(8, 99, 64)},
                             {QColor(17, 43, 192), QColor(8, 19, 131)},
                             {QColor(102, 67, 27), QColor(87, 35, 21)},
                             {QColor(193, 17, 17), QColor(120, 8, 57)},
                             {QColor(62, 71, 78), QColor(30, 30, 38)},
                             {QColor(244, 244, 86), QColor(194, 134, 34)}};

// should make a function to get new player
InGameUI::InGameUI(/*QString nickname, */QLabel *parent) : QLabel(parent), currentTask(nullptr), qLabel(nullptr), gameMap(nullptr)
{
    // doing this at the very first window would be nice (when asking nickname etc)
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    /*tasks = { // couldn't put all not necessary stuff in initialize not to delay user input ?
        new Task(TASK_FIX_WIRING, QPoint(4060, 360)),
        new Task(TASK_FIX_WIRING, QPoint(5433,2444)),
        new Task(TASK_FIX_WIRING, QPoint(7455,2055)),
        new Task(TASK_ASTEROIDS,  QPoint(6653, 900))
    };*/
    /*tasksLocations[TASK_FIX_WIRING] = {QPoint(4060, 360), QPoint(5433,2444),QPoint(7455,2055)};
    tasksLocations[TASK_ASTEROIDS] = {QPoint(6653,900)};*/
    readyButtonLayout = nullptr;
    // FOR TESTING
    /*currPlayer.isImpostor = true;
    currPlayer.isGhost = true;
    currPlayer.showBody = true;
    currPlayer.bodyX = currPlayer.x;
    currPlayer.bodyY = currPlayer.y;*/
}

void InGameUI::initialize(QString nickname)
{
    //quint16 otherPlayersSize = otherPlayers.size(); // used to be in constructor but likewise couldn't have already access to other players
    quint8 playersNumber = getPlayersNumber();
    //qInfo(("playersNumber: " + QString::number(playersNumber)).toStdString().c_str());
    currPlayer = Player(X_SPAWN, Y_SPAWN, nickname, colors[playersNumber][0], colors[playersNumber][1]);
    everyoneReady = false;
    lastNx = 0;
    lastNy = 0;
    //otherPlayers.push_back(Player(X_SPAWN+200, Y_SPAWN, "Test player", colors[0][0], colors[0][1]));
    // FOR TESTING
    /*otherPlayers[0].isGhost = true;
    otherPlayers[0].showBody = true;
    otherPlayers[0].bodyX = otherPlayers[0].x;
    otherPlayers[0].bodyY = otherPlayers[0].y;*/
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
    return !currPlayer.isGhost && collisionImage.pixelColor(x, y) == QColor(255, 0, 0);
}

void InGameUI::initDisplay()
{
    backgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"
    collisionPixmap = getQPixmap("mapCropCollision.png");
    collisionImage = collisionPixmap->toImage();
    QPixmap* killButtonPixmap = getQPixmap("killButton.png");
    killButtonImage = killButtonPixmap->toImage();
    QPixmap* reportButtonPixmap = getQPixmap("reportButton.png");
    reportButtonImage = reportButtonPixmap->toImage();
    QPixmap* useButtonPixmap = getQPixmap("useButton.png");
    useButtonImage = useButtonPixmap->toImage();
    QPixmap* playAgainButtonPixmap = getQPixmap("playAgainButton.png");
    playAgainButtonImage = playAgainButtonPixmap->toImage();

    if (isCollision(currPlayer.x, currPlayer.y))
    {
        bool found = false;
        for (currPlayer.x = 0; currPlayer.x < backgroundPixmap->size().width() && !found; currPlayer.x++)
            for (currPlayer.y = 0; currPlayer.y < backgroundPixmap->size().height() && !found; currPlayer.y++)
                if (!isCollision(currPlayer.x, currPlayer.y))
                    found = true;
    }
    //setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

/**
 * Displays given pixmap centered at map coordinates (centerx, centery).
 * Uses the provided painter if any.
 */
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

/**
 * Centers the in-game UI at the given coordinates, clipping at borders if necessary.
 * If the map is smaller than the window in some dimension, the map is centered along that dimension.
 * @brief InGameUI::setCenterBorderLimit
 * @param x
 * @param y
 * @param painter A QPainter that will be used for painting the background.
 * If nullptr is passed, a new QPainter will be used instead.
 */
void InGameUI::setCenterBorderLimit(int x, int y, QPainter *painter = nullptr)
{
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
    if(painter)
        painter->drawPixmap(leftBackground, topBackground, *backgroundPixmap);
    else {
        QPainter locPainter(windowPixmap);
        locPainter.drawPixmap(leftBackground, topBackground, *backgroundPixmap);
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
bool InGameUI::performMovement(qint64 elapsed, int dirVert, int dirHoriz) // seems to be executed even if not using movement keys... leading to existence of lastNx/y
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
        if(lastNx == 0 && lastNy == 0)
        {
            lastNx = nx;
            lastNy = ny;
        }
        if(lastNx != nx || lastNy != ny)
        {
            sendToAll("Position " + QString::number(nx) + " " + QString::number(ny));
            lastNx = nx;
            lastNy = ny;
        }
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
    sort(players.begin(), players.end(), [&](const Player *a, const Player *b) {
        int sqDistA = (a->x-x) * (a->x-x) + (a->y-y) * (a->y-y);
        int sqDistB = (b->x-x) * (b->x-x) + (b->y-y) * (b->y-y);
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
    return !currPlayer.isGhost && (distanceToEmergencyButton() < EMERGENCY_RANGE_SQUARED);
}

quint64 InGameUI::distanceToEmergencyButton()
{
    return qPow(currPlayer.x - EMERGENCY_BUTTON_X, 2) + qPow(currPlayer.y - EMERGENCY_BUTTON_Y, 2);
}

QVector<Task*> InGameUI::getUsableTasksByDistance() {
    QVector<Task*> ret;
    int x = currPlayer.x, y = currPlayer.y;
    for(Task* task : tasks) {
        QPoint pt = task->location;
        if(task->finished)
            continue;
        int dist = (pt.x()-x)*(pt.x()-x) + (pt.y()-y)*(pt.y()-y);
        if(dist <= TASK_RANGE_SQUARED)
            ret.push_back(task);
    }
    sort(ret.begin(), ret.end(), [&](const Task* task1, const Task* task2) {
        QPoint pt1 = task1->location;
        QPoint pt2 = task2->location;
        int dist1 = (pt1.x()-x)*(pt1.x()-x) + (pt1.y()-y)*(pt1.y()-y);
        int dist2 = (pt2.x()-x)*(pt2.x()-x) + (pt2.y()-y)*(pt2.y()-y);
        if(dist1 != dist2)
            return dist1 < dist2;
        else if(pt1.x() != pt2.x())
            return pt1.x() < pt2.x();
        else
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
            // used to use player->x/y instead of bodyX/Y
            int sqDist = qPow(player->bodyX-x, 2) + qPow(player->bodyY-y, 2);
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
    // TODO
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
    if(showGhost && (!currPlayer.isGhost || !player.isGhost))
    {
        //qInfo("stoped");
        return;
    }
    else if(player.isGhost && !player.showBody && !showGhost)
        displayPlayer(player, painter, true);
    else {
        int x = forceX ? forceX : (player.isGhost && !showGhost) ? player.bodyX : player.x;
        int y = forceY ? forceY : (player.isGhost && !showGhost) ? player.bodyY : player.y;
        QPixmap* toDraw;
        if(!showGhost) {
            if(player.isGhost)
                toDraw = player.deadPixmap;
            else if(player.playerFacingLeft)
                toDraw = player.flippedPixmap;
            else
                toDraw = player.playerPixmap;
        }
        else {
            if(player.playerFacingLeft)
                toDraw = player.flippedGhostPixmap;
            else
                toDraw = player.ghostPixmap;
        }
        //qInfo((QString::number(player.isGhost) + " " + QString::number(y - toDraw->size().height() / 2)).toStdString().c_str());
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
        if(!showGhost)
            displayPlayer(player, newPainter, true);
        if (!painter)
            delete newPainter;
    }
}

/**
 * Performs player movement, then redraws the in-game UI. Meant to be called at each frame.
 * @brief InGameUI::redraw
 */
void InGameUI::redraw()
{
    if(currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES || currentInGameGUI == IN_GAME_GUI_WIN_IMPOSTORS)
    {
        QPixmap *oldPixmap = windowPixmap;
        QSize qSize = size();
        windowPixmap = new QPixmap(qSize);
        QPainter painter(windowPixmap);
        painter.fillRect(0, 0, qSize.width(), qSize.height(), Qt::black);
        QString winningTeam = currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES ? "crewmates" : "impostors";
        painter.setPen(currentInGameGUI == IN_GAME_GUI_WIN_CREWMATES ? Qt::blue : Qt::red);
        QString title = firstUppercase(QString("%1's victory").arg(winningTeam));
        painter.setFont(QFont("arial", 25));
        QFontMetrics fm(painter.font());
        quint16 middleX = qSize.width() / 2, middleY = qSize.height() / 2;
        painter.drawText(middleX - fm.width(title) / 2, qSize.height() / 10, title);

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
            //qInfo(("winner: " + player->nickname + " " + QString::number(middleX) + " " + QString::number(middleY) + " " + QString::number(qSize.width()) + " " + QString::number(qSize.height())).toStdString().c_str());
            displayPlayer(*player, &painter, false/*true*/, middleX + 50 * playersIndex, middleY);
        }
        painter.drawImage(qSize.width()-130, qSize.height()-130, playAgainButtonImage);

        setPixmap(*windowPixmap);
        delete oldPixmap;
        return;
    }
    // Movement
    qint64 now = elapsedTimer->elapsed();
    qint64 elapsed = now - lastUpdate;
    lastUpdate = now;
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

    QPixmap *oldPixmap = windowPixmap;
    windowPixmap = new QPixmap(size());
    QPainter painter(windowPixmap);
    setCenterBorderLimit(currPlayer.x, currPlayer.y - currPlayer.playerPixmap->size().height() / 2, &painter);
    // Display players with ascending y, then ascending x
    QVector<Player *> players;
    players.push_back(&currPlayer);
    for (Player &player : otherPlayers)
        players.push_back(&player);
    sort(players.begin(), players.end(), [](const Player *a, const Player *b)
          {
              bool isCurrPlayerGhost = inGameUI->currPlayer.isGhost;
              int aY = isCurrPlayerGhost ? a->y : a->bodyY, // used to use a->isGhost
                  aX = isCurrPlayerGhost ? a->x : a->bodyX,
                  bY = isCurrPlayerGhost ? b->y : b->bodyY,
                  bX = isCurrPlayerGhost ? b->x : b->bodyX;
              if(aY != bY)
                  return aY < bY;
              else if(aX != bX)
                  return aX < bX;
              return a->nickname.compare(b->nickname) < 0;
          });
    for (Player *player : players)
        displayPlayer(*player, &painter);

    int fontSizePt = 23;
    // Impostor message
    if(currPlayer.isImpostor) {
        painter.setFont(QFont("Liberation Sans", fontSizePt));
        QRect textRect(0, 0, 1, fontSizePt);
        QRect boundingRect;
        //QPen oldPen = painter.pen();
        painter.setPen(Qt::red);
        painter.drawText(textRect, Qt::TextDontClip | Qt::AlignLeft, "You are an Impostor!", &boundingRect);
        painter.fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
        painter.drawText(textRect, Qt::TextDontClip | Qt::AlignLeft, "You are an Impostor!");
        //painter.setPen(oldPen);
    }

    // For debugging purposes: show current location - should make a boolean to choose whether or not to display location
    /*QRect textRect(size().width()-1, 0, 1, fontSizePt);
    QRect boundingRect;
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y), &boundingRect);
    boundingRect.setLeft(size().width()-1-boundingRect.width());
    boundingRect.setRight(size().width()-1);
    painter.fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
    painter.drawText(boundingRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y));*/

    // Game buttons
    if(everyoneReady && findKillablePlayer())
        painter.drawImage(size().width()-220, size().height()-110, killButtonImage);
    if(findReportableBody())
        painter.drawImage(size().width()-110, size().height()-110, reportButtonImage);
    if(everyoneReady && (getUsableTasksByDistance().size() > 0) || isNearEmergencyButton())
        painter.drawImage(size().width()-110, size().height()-220, useButtonImage);

    // Ready button
    if (!everyoneReady)
    {
        if(!readyButtonLayout) {
            qDebug() << "Creating ready button";
            readyButtonLayout = new QGridLayout;
            readyButton = new QPushButton("Ready");
            readyButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
            connect(readyButton, &QPushButton::released, this, &InGameUI::onReadyClicked);
            readyButtonLayout->addWidget(readyButton, 0, 0, Qt::AlignBottom | Qt::AlignRight);
            setLayout(readyButtonLayout);
        }
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
            readyButton->setText("Waiting for other players");
            sendToAll("ready");
            checkEverybodyReady();
        }
        else
        {
            qDebug() << "Still waiting for players..."; // could display it on the button by default how many we are waiting
        }
    }
}

void InGameUI::setImposter(QString nickname)
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
        setImposter(nickname);
        sendToAll("Imposter " + nickname);
    }
    readyButtonLayout->removeWidget(readyButton);
    delete readyButton;
    delete readyButtonLayout;
    readyButtonLayout = nullptr;

    QList<QString> peerAddresses = otherPlayers.keys();
    quint8 peerAddressesSize = peerAddresses.size();
    for(quint8 peerAddressesIndex = 0; peerAddressesIndex < peerAddressesSize; peerAddressesIndex++)
    {
        QString peerAddress = peerAddresses[peerAddressesIndex];
        movePlayer(peerAddress, X_SPAWN, Y_SPAWN, true);
    }
    currPlayer.x = X_SPAWN;
    currPlayer.y = Y_SPAWN;
    currPlayer.playerFacingLeft = false;
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
    quint8 alivePlayersNumber = currPlayer.isGhost ? 0 : 1;
    for(Player& player : otherPlayers)
        if(!player.isGhost)
            alivePlayersNumber++;
    return alivePlayersNumber;
}

// see voted for the moment would be nice
void InGameUI::executeVote(QString voteStr)
{
    meetingWidget->waitingVotes--;
    if(voteStr != "Skip")
    {
        meetingWidget->votes[voteStr.replace("Voted ", "")]++;
    }
    if(meetingWidget->waitingVotes == 0)
    {
        QList<QString> nicknames = meetingWidget->votes.keys();
        quint8 maxVotes = 0, nicknamesSize = nicknames.size();
        QString nicknameMostVoted = "";
        bool exAequo = false;
        for(quint8 nicknamesIndex = 0; nicknamesIndex < nicknamesSize; nicknamesIndex++)
        {
            QString nickname = nicknames[nicknamesIndex];
            quint8 currentVotes = meetingWidget->votes[nickname];
            if(currentVotes > maxVotes)
            {
                maxVotes = currentVotes;
                nicknameMostVoted = nickname;
                exAequo = false;
            }
            else if(currentVotes == maxVotes && currentVotes > 0)
            {
                exAequo = true;
            }
        }
        if(nicknameMostVoted != "" && !exAequo)
        {
            killPlayer(*getPlayer(nicknameMostVoted));
        }
        closeMeetingUI();
        checkEndOfTheGame();
    }
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

void InGameUI::checkEndOfTheGame() // could optimize by precising which checkEndOfTheGame (task or death) to optimize
{
    if(gameCommonTasks + gameLongTasks + gameShortTasks == 0)
    {
        qInfo("all crewmates tasks done !");
        currentInGameGUI = IN_GAME_GUI_WIN_CREWMATES;
    }
    else
    {
        if(getAliveCrewmatesNumber() == 0)
        {
            qInfo("impostors win !");
            currentInGameGUI = IN_GAME_GUI_WIN_IMPOSTORS;
        }
        else if(getAliveImpostorsNumber() == 0)
        {
            qInfo("crewmates win !");
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
        default:
        break;
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
        default:
            return;
        }
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
    // Game logic verifications are performed in the function calls
    Player* killable = findKillablePlayer();
    if(killable)
    {
        killPlayer(*killable);
        inGameUI->checkEndOfTheGame();
        sendToAll("Kill " + killable->nickname);
    }
}

void InGameUI::openMap() {
    if(gameMap || currentInGameGUI != IN_GAME_GUI_NONE || !everyoneReady)
        return;
    gameMap = new GameMap(this);
    currHLayout = new QHBoxLayout;
    currHLayout->addStretch();
    currHLayout->addWidget(gameMap);
    currHLayout->addStretch();
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
    // TODO: networking
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
    if(reportedPlayer)
        reportedPlayer->showBody = false;
    meetingWidget = new MeetingUI(this, reportedPlayer, reportingPlayer);

    currHLayout = makeCenteredLayout(meetingWidget);;
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
            else if(qLabel)
                closeTask();
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
    default:
        break;
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
        default:
            break;
        }
    }
    QLabel::keyReleaseEvent(key);
}

void InGameUI::mouseMoveEvent(QMouseEvent *mouseEvent) {
    if (qLabel != nullptr && mouseEvent->buttons() & Qt::LeftButton)
    {
        if (currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
            onMouseEventFixWiring(mouseEvent);
    }
    QLabel::mouseMoveEvent(mouseEvent);
}

void InGameUI::mousePressOrDoubleClick(QMouseEvent *mouseEvent) {
    if(everyoneReady && mouseEvent->button() == Qt::LeftButton) {
        if(currentInGameGUI == IN_GAME_GUI_NONE) {
            int mouseX = mouseEvent->x(), mouseY = mouseEvent->y();
            int width = size().width(), height = size().height();
            if(mouseX >= width-220 && mouseX < width-110 && mouseY >= height-110 && mouseY < height && findKillablePlayer())
                onClickKill();
            else if(mouseX >= width-110 && mouseX < width && mouseY >= height-110 && mouseY < height && findReportableBody())
                onClickReport();
            else if(mouseX >= width-110 && mouseX < width && mouseY >= height-220 && mouseY < height-110 && ((getUsableTasksByDistance().size() > 0) || isNearEmergencyButton()))
                onClickUse();
        }
        else if(currentInGameGUI == IN_GAME_GUI_ASTEROIDS) {
            onMouseEventAsteroids(mouseEvent);
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
    //qInfo(("gettingPlayersNumber: " + QString::number(otherPlayers.size()) + " " + currPlayer.nickname + " ! " + QString::number(currPlayer.nickname == "" ? 0 : 1)).toStdString().c_str());
    return otherPlayers.size() + (currPlayer.nickname == "" ? 0 : 1);
}

void InGameUI::spawnOtherPlayer(QString peerAddress, QString otherPlayerNickname)
{
    //int otherPlayersSize = otherPlayers.size()/* + 1*/;
    //otherPlayers.push_back(Player(X_SPAWN, Y_SPAWN, otherPlayerNickname, colors[otherPlayersSize][0], colors[otherPlayersSize][1]));
    quint8 playersNumber = getPlayersNumber();
    otherPlayers[peerAddress] = Player(X_SPAWN, Y_SPAWN, otherPlayerNickname, colors[playersNumber][0], colors[playersNumber][1]);
}

void InGameUI::setFacingLeftPlayer(QString peerAddress)
{
    otherPlayers[peerAddress].playerFacingLeft = true;
}

void InGameUI::movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp)
{
    /*QList<QString> peerAddresses = otherPlayers.keys();
    quint8 peerAddressesSize = peerAddresses.size();
    qInfo(("peerAddress: " + peerAddress).toStdString().c_str());
    for(quint8 peerAddressesIndex = 0; peerAddressesIndex < peerAddressesSize; peerAddressesIndex++)
    {
        QString peerAddress = peerAddresses[peerAddressesIndex];
        qInfo((QString::number(peerAddressesIndex) + " " + peerAddress).toStdString().c_str());
    }
    Player* player = &otherPlayers[peerAddress];
    qInfo(("moving: " + player->nickname + " !").toStdString().c_str());*/
    Player* player = &otherPlayers[peerAddress];
    if(tp)
        player->playerFacingLeft = false;
    else if(x != player->x) // otherwise if just change vertically not logical
        player->playerFacingLeft = x < player->x;
    player->x = x;
    player->y = y;
    /*player->bodyX = x;
    player->bodyY = y;*/
}

void InGameUI::checkEverybodyReady(bool threadSafe)
{
    //qInfo("a");
    if(!currPlayer.isReady) return;
    //qInfo("b");
    QList<QString> peerAddresses = otherPlayers.keys();
    quint8 peerAddressesSize = peerAddresses.size();
    for(quint8 peerAddressesIndex = 0; peerAddressesIndex < peerAddressesSize; peerAddressesIndex++)
    {
        QString peerAddress = peerAddresses[peerAddressesIndex];
        if(!otherPlayers[peerAddress].isReady)
        {
            //qInfo((peerAddress + " " + QString::number(peerAddressesSize)).toStdString().c_str());
            return;
        }
    }
    //qInfo("c");
    onEverybodyReady(threadSafe);
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
    quint8 peerAddressesSize = peerAddresses.size();
    for(quint8 peerAddressesIndex = 0; peerAddressesIndex < peerAddressesSize; peerAddressesIndex++)
    {
        QString peerAddress = peerAddresses[peerAddressesIndex];
        Player* player = &otherPlayers[peerAddress];
        if(player->nickname == nickname)
            return player;
    }
    return nullptr;
}
