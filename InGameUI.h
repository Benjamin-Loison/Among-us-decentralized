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

const int FPS = 30;

class InGameUI : public QLabel
{
    Q_OBJECT

    private:
        Player currPlayer;
        QVector<Player> otherPlayers;
        int topBackground, leftBackground;
        bool everyoneReady;
        QPixmap* backgroundPixmap;
        QPixmap* collisionPixmap;
        QImage collisionImage;
        QPixmap* windowPixmap;
        QTimer* timer;
        QElapsedTimer* elapsedTimer;
        QMap<int, bool> isPressed;
        qint64 lastUpdate;
        QHBoxLayout* currLayout;
        QGridLayout* readyButtonLayout;
        QPushButton* readyButton;

    public:
        QLabel* qLabel;
        void initialize();
        void initDisplay();
        void displayAt(QPixmap *pixmap, int centerx, int centery, QPainter* painter);
        void displayPlayer(const Player &player, QPainter* painter);
        bool performMovement(qint64 elapsed, int dirVert, int dirHoriz);
        void setCenterBorderLimit(int x, int y, QPainter* painter);
        bool isCollision(quint16 x, quint16 y);
        void resizeEvent(QResizeEvent* ev);
        InGameUI(QString nickname, QLabel* parent = 0);

    public slots:
        void redraw();
        void onReadyClicked();

    protected:
        bool eventFilter(QObject* obj, QEvent* event);
};

#endif
