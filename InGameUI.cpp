#include "InGameUI.h"
#include "main.h"
#include "asteroids.h"
#include <QPushButton>
#include <algorithm>
using namespace std;

const int MOVEMENT_SPEED_SEC = 477;
const int X_SPAWN = 5500;
const int Y_SPAWN = 1100;
const int KILL_RANGE_SQUARED = 200*200; // 200 pixels
const int TASK_RANGE_SQUARED = 200*200; // 200 pixels
const int REPORT_RANGE_SQUARED = 200*200; // 200 pixels
const QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)},
             colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                             {QColor(20, 156, 20), QColor(8, 99, 64)},
                             {QColor(17, 43, 192), QColor(8, 19, 131)},
                             {QColor(102, 67, 27), QColor(87, 35, 21)},
                             {QColor(193, 17, 17), QColor(120, 8, 57)},
                             {QColor(62, 71, 78), QColor(30, 30, 38)},
                             {QColor(244, 244, 86), QColor(194, 134, 34)}};

// should make a function to get new player
InGameUI::InGameUI(/*QString nickname, */QLabel *parent) : QLabel(parent), currentTask(nullptr), qLabel(nullptr)
{
    // doing this at the very first window would be nice (when asking nickname etc)
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");
    tasks = { // couldn't put all not necessary stuff in initialize not to delay user input ?
        new Task(TASK_FIX_WIRING, QPoint(4060, 360)),
        new Task(TASK_FIX_WIRING, QPoint(5433,2444)),
        new Task(TASK_FIX_WIRING, QPoint(7455,2055)),
        new Task(TASK_ASTEROIDS,  QPoint(6653, 900))
    };
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
            sendToAll("position " + QString::number(nx) + " " + QString::number(ny));
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
    qSort(players.begin(), players.end(), [&](const Player *a, const Player *b) {
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
    qSort(ret.begin(), ret.end(), [&](const Task* task1, const Task* task2) {
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
    // Confirm: ghosts can't report bodies, right?
    if(currPlayer.isGhost)
        return nullptr;
    int x = currPlayer.x, y = currPlayer.y;
    for(Player* player : getOtherPlayersByDistance()) {
        if(player->showBody) {
            int sqDist = (player->x-x)*(player->x-x) + (player->y-y)*(player->y-y);
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
    // TODO
    qDebug() << "Reported player" << p.nickname;
    p.showBody = false;
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
            int sqDist = (player->x-x)*(player->x-x) + (player->y-y)*(player->y-y);
            if(sqDist <= KILL_RANGE_SQUARED)
                return player;
            else
                break;
        }
    }
    return nullptr;
}

/**
 * Kills the given Player.
 */
bool InGameUI::killPlayer(Player &p) {
    if(!currPlayer.isImpostor || p.isGhost)
        return false;
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
void InGameUI::displayPlayer(const Player &player, QPainter *painter = nullptr, bool showGhost = false)
{
    // Only ghosts see ghosts
    if(showGhost && (!currPlayer.isGhost || !player.isGhost))
        return;
    else if(player.isGhost && !player.showBody && !showGhost)
        displayPlayer(player, painter, true);
    else {
        int x = (player.isGhost && !showGhost) ? player.bodyX : player.x;
        int y = (player.isGhost && !showGhost) ? player.bodyY : player.y;
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
        displayAt(toDraw, x, y - toDraw->size().height() / 2, painter);
        QPainter *newPainter;
        if (painter)
            newPainter = painter;
        else
            newPainter = new QPainter(windowPixmap);
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
    qSort(players.begin(), players.end(), [](const Player *a, const Player *b)
          {
              if (a->y != b->y)
                  return a->y < b->y;
              else
                  return a->x < b->x;
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

    // For debugging purposes: show current location
    QRect textRect(size().width()-1, 0, 1, fontSizePt);
    QRect boundingRect;
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y), &boundingRect);
    boundingRect.setLeft(size().width()-1-boundingRect.width());
    boundingRect.setRight(size().width()-1);
    painter.fillRect(boundingRect, QBrush(QColor(128, 128, 128, 128)));
    painter.drawText(boundingRect, Qt::TextDontClip | Qt::AlignRight, QString("Location: %1, %2").arg(currPlayer.x).arg(currPlayer.y));

    // Game buttons
    if(findKillablePlayer())
        painter.drawImage(size().width()-220, size().height()-110, killButtonImage);
    if(findReportableBody())
        painter.drawImage(size().width()-110, size().height()-110, reportButtonImage);
    if(getUsableTasksByDistance().size() > 0)
        painter.drawImage(size().width()-110, size().height()-220, useButtonImage);

    // Ready button
    if (!everyoneReady)
    {
        if(!readyButtonLayout) {
            qDebug() << "Creating ready button";
            readyButtonLayout = new QGridLayout;
            readyButton = new QPushButton("Ready");
            readyButton->installEventFilter(this);
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
    if(!currPlayer.isReady)
    {
        if(getPlayersNumber() >= MINIMAL_NUMBER_OF_PLAYERS)
        {
            qDebug() << "Ready clicked";
            currPlayer.isReady = true;
            readyButton->setText("Waiting other players");
            sendToAll("ready");
            checkEverybodyReady();
        }
        else
        {
            qDebug() << "Still waiting some players..."; // could display it on the button by default how many we are waiting
        }
    }
}

void InGameUI::onEverybodyReady()
{
    everyoneReady = true;
    readyButtonLayout->removeWidget(readyButton);
    readyButton->removeEventFilter(this);
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

void InGameUI::finishTask() {
    if(!currentTask)
        return;
    currentTask->finished = true;
}

void InGameUI::closeTask() {
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
    qLabel->removeEventFilter(this);
    delete currLayout;
    delete qLabel;
    qLabel = nullptr;
}

void InGameUI::onClickUse() {
    QVector<Task*> usableTasks = getUsableTasksByDistance();
    if(usableTasks.size() > 0) {
        Task* task = usableTasks[0];
        switch(task->taskType) {
        case TASK_FIX_WIRING:
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_FIX_WIRING;
            qLabel = getFixWiring(this);
            break;
        case TASK_ASTEROIDS:
            currentTask = task;
            currentInGameGUI = IN_GAME_GUI_ASTEROIDS;
            qLabel = getAsteroids(elapsedTimer->elapsed(), this);
            break;
        default:
            return;
        }
        currLayout = new QHBoxLayout;
        currLayout->addWidget(qLabel);
        setLayout(currLayout);
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
        killPlayer(*killable);
}

/**
 * Filters key presses used in the game, and mouse events for tasks. (there may be a more efficient implementation)
 * @brief InGameUI::eventFilter
 * @param obj The object that received the event.
 * @param event The event.
 * @return false to pass the event to further filters and obj, true to stop processing the event
 */
bool InGameUI::eventFilter(QObject *obj, QEvent *event)
{
    /*if(obj != parent())
        return false;*/
    if (event->type() == QEvent::KeyPress)
    {
        qDebug() << "Key press sent to object of type" << obj->metaObject()->className();
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
                        onClickUse();
                    }
                    else
                    {
                        closeTask();
                    }
                }
                break;
            case Qt::Key_K:
                if(everyoneReady && currentInGameGUI == IN_GAME_GUI_NONE) {
                    onClickKill();
                }
                break;
            case Qt::Key_R:
                if(everyoneReady && currentInGameGUI == IN_GAME_GUI_NONE) {
                    onClickReport();
                }
                break;
            default:
                break;
            }
        }
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
                break;
            }
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (qLabel != nullptr && mouseEvent->buttons() & Qt::LeftButton)
        {
            if (currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
                onMouseEventFixWiring(mouseEvent);
        }
    }
    else if (event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseButtonPress)
    {
        if(!everyoneReady || obj != this)
            return false;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        
        if(mouseEvent->button() == Qt::LeftButton) {
            int mouseX = mouseEvent->x(), mouseY = mouseEvent->y();
            int width = size().width(), height = size().height();
            if(currentInGameGUI == IN_GAME_GUI_NONE) {
                if(mouseX >= width-220 && mouseX < width-110 && mouseY >= height-110 && mouseY < height && findKillablePlayer())
                    onClickKill();
                else if(mouseX >= width-110 && mouseX < width && mouseY >= height-110 && mouseY < height && findReportableBody())
                    onClickReport();
                else if(mouseX >= width-110 && mouseX < width && mouseY >= height-220 && mouseY < height-110 && getUsableTasksByDistance().size() > 0)
                    onClickUse();
                return false;
            }
            else if(currentInGameGUI == IN_GAME_GUI_ASTEROIDS) {
                onMouseEventAsteroids(mouseEvent);
            }
        }
    }
    return false;
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

void InGameUI::movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp)
{
    /*QList<QString> peerAddresses = otherPlayers.keys();
    quint8 peerAddressesSize = peerAddresses.size();
    qInfo(("peerAddress: " + peerAddress).toStdString().c_str());
    for(quint8 peerAddressesIndex = 0; peerAddressesIndex < peerAddressesSize; peerAddressesIndex++)
    {
        QString peerAddress = peerAddresses[peerAddressesIndex];
        qInfo((QString::number(peerAddressesIndex) + " " + peerAddress).toStdString().c_str());
    }*/
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

void InGameUI::checkEverybodyReady()
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
    onEverybodyReady();
}

void InGameUI::setPlayerReady(QString peerAddress)
{
    Player* player = &otherPlayers[peerAddress];
    player->isReady = true;
    checkEverybodyReady();
}
