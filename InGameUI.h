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
#include "qPlus.h"
#include "Player.h"
#include "Task.h"

const int FPS = 30;

class InGameUI : public QLabel
{
    Q_OBJECT

    private:
        QMap<QString, Player> otherPlayers;
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
        QHBoxLayout* currLayout;
        QGridLayout* readyButtonLayout;
        QPushButton* readyButton;
        Task* currentTask;

    public:
        Player currPlayer; // used to be private
        QLabel* qLabel;
        void initialize(QString nickname);
        void initDisplay();
        void displayAt(QPixmap *pixmap, int centerx, int centery, QPainter* painter);
        void displayPlayer(const Player &player, QPainter* painter, bool showGhost);
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
        void spawnOtherPlayer(QString peerAddress, QString otherPlayerNickname);
        void movePlayer(QString peerAddress, quint32 x, quint32 y, bool tp = false);
        void onEverybodyReady();
        void checkEverybodyReady();
        void setPlayerReady(QString peerAddress);
        quint8 getPlayersNumber();
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
