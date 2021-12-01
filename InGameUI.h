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
#include <QTimer>
#include <QString>
#include <QElapsedTimer>
#include <QPushButton>
#include "fixWiring.h"
#include "GameMap.h"
#include "meetings.h"
#include "Player.h"
#include "qPlus.h"
#include "Task.h"
#include <QThread>

#define X_SPAWN 5500
#define Y_SPAWN 1100

const int FPS = 30;

class GameMap;

class InGameUI : public QLabel
{
    Q_OBJECT

    private:
        QVector<Task*> tasks;
        int topBackground, leftBackground;
        quint32 lastNx, lastNy;
        bool everyoneReady;
        QPixmap* backgroundPixmap;
        QPixmap* collisionPixmap;
        QImage collisionImage;
        QImage killButtonImage;
        QImage reportButtonImage;
        QImage useButtonImage;
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

    public:
        Player currPlayer; // used to be private
        QMap<QString, Player> otherPlayers; //same
        QLabel* qLabel;
        QWidget* meetingWidget; // may be merged with qLabel
        void initialize(QString nickname);
        void initDisplay();
        void displayAt(QPixmap *pixmap, int centerx, int centery, QPainter* painter);
        void displayPlayer(const Player &player, QPainter *painter = nullptr, bool showGhost = false, quint16 forceX = 0, quint16 forceY = 0);
        bool performMovement(qint64 elapsed, int dirVert, int dirHoriz);
        void setCenterBorderLimit(int x, int y, QPainter* painter);
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
        void triggerMeeting(Player* reportedPlayer); // Called by the client who reports the body
        void openMeetingUI(Player* reportedPlayer = nullptr, Player* reportingPlayer = nullptr); // Also called by other clients upon receipt of the corresponding packet
        void closeMeetingUI();
        void spawnOtherPlayer(QString peerAddress, QString otherPlayerNickname);
        void movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp = false);
        void setFacingLeftPlayer(QString peerAddress);
        void onEverybodyReady(bool threadSafe);
        void onEverybodyReadySub(bool threadSafe);
        void checkEverybodyReady(bool threadSafe = false);
        void setPlayerReady(QString peerAddress, bool threadSafe = false);
        void checkEndOfTheGame();
        void taskFinished(TaskTime taskTime);
        quint8 getAliveCrewmatesNumber();
        quint8 getAliveImpostorsNumber();
        quint8 getPlayersNumber();
        QPixmap* getBackgroundPixmap();
        QVector<Task*> getTasks();
        Player* getPlayer(QString nickname);
        void setImposter(QString nickname);
        quint8 waitingAnswersNumber;
        quint8 gameCommonTasks, gameLongTasks, gameShortTasks;
        InGameUI(/*QString nickname,*/ QLabel* parent = 0);

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
