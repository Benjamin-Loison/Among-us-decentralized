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
#include "fixWiring.h"
#include "qPlus.h"

const int FPS = 30;

class QLabelKeys : public QLabel
{
    Q_OBJECT

    private:
        quint16 x, y;
        quint16 topBackground, leftBackground;
        QPixmap* playerPixmap;
        QPixmap* backgroundPixmap;
        QPixmap* windowPixmap;
        QTimer* timer;
        QMap<int, bool> isPressed;

    public:
        QLabel* qLabel;
        void display();
        void displayAt(QPixmap *pixmap, int centerx, int centery);
        void setCenterBorderLimit(int x, int y);
        void resizeEvent(QResizeEvent* ev);
        QLabelKeys(QLabel* parent = 0);

    public slots:
        void redraw();

    protected:
        bool eventFilter(QObject* obj, QEvent* event);
};

#endif
