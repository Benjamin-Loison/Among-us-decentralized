#ifndef INGAMEUI_H
#define INGAMEUI_H

#include <QLabel>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QRandomGenerator>
#include <QTimer>
#include <QString>
#include <QElapsedTimer>
#include <QPushButton>
#include "Door.h"
#include "fixWiring.h"
#include "GameMap.h"
#include "meetings.h"
#include "Player.h"
#include "qPlus.h"
#include "Room.h"
#include "Task.h"
#include "Vents.h"
#include <QThread>
#include "EnterIDCode.h"
#include "AlignEngine.h"

#define EMERGENCY_BUTTON_X 4830
#define EMERGENCY_BUTTON_Y 1045

const int FPS = 30;

class Door;
class GameMap;

class InGameUI : public QLabel
{
    Q_OBJECT

    private:
        QVector<Task*> tasks;
        int topBackground, leftBackground;
        bool everyoneReady;
        QPixmap* backgroundPixmap;
        QPixmap* collisionPixmap;
        QImage collisionImage;
        QImage killButtonImage;
        QImage reportButtonImage;
        QImage useButtonImage;
        QImage playAgainButtonImage;
        QImage ventArrowImage;
        QPixmap* windowPixmap;
        QTimer* timer;
        QElapsedTimer* elapsedTimer;
        QMap<int, bool> isPressed;
        qint64 lastUpdate;
        QHBoxLayout* currHLayout;
        QGridLayout* readyButtonLayout;
        QPushButton* readyButton;
        Task* currentTask;
        GameMap* gameMap;
        QString privateRandom;
        QPainter* killButtonPainter;
        qint64 lastKillTime;
        bool initialized;
        bool isPolus;

    public:
        QVector<Room> rooms;
        QVector<Door> doors; // temporarily public
        Player currPlayer; // used to be private
        QMap<QString, Player> otherPlayers; //same
        QLabel *qLabel;
        MeetingUI *meetingWidget; // may be merged with qLabel
        MeetingResultsUI *meetingResultsWidget;
        void initialize(QString nickname, bool Polus);
        void initDisplay();
        void initDoorsAndRooms();
        qint64 currTimer();
        void displayAt(QPixmap *pixmap, int centerx, int centery, QPainter* painter);
        void displayPlayer(const Player &player, QPainter *painter = nullptr, bool showGhost = false, quint16 forceX = 0, quint16 forceY = 0);
        bool performMovement(qint64 elapsed, int dirVert, int dirHoriz);
        void setCenterBorderLimit(int x, int y, QPainter* painter = nullptr, QSize s = QSize(), quint16 offsetX = 0, quint16 offsetY = 0, quint16 sx = 0, quint16 sy = 0);
        bool isCollision(quint16 x, quint16 y);
        void resizeEvent(QResizeEvent* ev);
        QVector<Player *> getOtherPlayersByDistance();
        QVector<Task *> getUsableTasksByDistance();
        Player* findKillablePlayer();
        bool killPlayer(Player &player);
        Player* findReportableBody();
        bool reportBody(Player &player);
        void finishTask();
        void closeTask();
        void onClickUse();
        void onClickReport();
        void onClickKill();
        void openMap();
        void closeMap();
        void closeVitals();
        void triggerMeeting(Player* reportedPlayer = nullptr); // Called by the client who reports the body
        void openMeetingUI(Player* reportedPlayer = nullptr, Player* reportingPlayer = nullptr); // Also called by other clients upon receipt of the corresponding packet
        void closeMeetingUI();
        void spawnOtherPlayer(QString peerAddress, QString otherPlayerNickname);
        void movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp = false);
        void setFacingLeftPlayer(QString peerAddress);
        void onEverybodyReady(bool threadSafe);
        void onEverybodyReadySub(bool threadSafe);
        void checkEverybodyReady(bool threadSafe = false);
        void setPlayerReady(QString peerAddress, bool threadSafe = false);
        void unreadyTeleportEveryone();
        void checkEndOfTheGame();
        void taskFinished(TaskTime taskTime);
        void executeVote(QString voteStr, Player *voter);
        void onAllProceeded(Player* whoToKill);
        quint8 getAlivePlayersNumber();
        quint8 getAliveCrewmatesNumber();
        quint8 getAliveImpostorsNumber();
        quint8 getPlayersNumber();
        QPixmap* getBackgroundPixmap();
        QVector<Task*> getTasks();
        Player* getPlayer(QString nickname);
        void setImpostor(QString nickname);
        quint8 waitingAnswersNumber;
        quint8 gameCommonTasks, gameLongTasks, gameShortTasks;
        quint64 distanceToEmergencyButton();
        bool isNearEmergencyButton();
        bool isThereAnyUsableTaskNear();
        void resetAllPlayers();
        void hidePlayerBodyIfDead(QString peerAddress);
        bool isWinScreen();
        QList<Player*> getAllPlayers();
        void teleportAllPlayers();
        void displayPlayers(QPainter* painter);
        void displayDoors(QPainter* painter);
        void setLayoutQLabel();
        InGameUI(QLabel* parent = 0);

    public slots:
        void redraw();
        void onReadyClicked();

    protected:
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressOrDoubleClick(QMouseEvent *event);
};

#endif
