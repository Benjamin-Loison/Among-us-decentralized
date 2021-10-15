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
#include "fixWiring.h"
#include "qPlus.h"

class QLabelKeys : public QLabel
{
    Q_OBJECT

    private:
        quint8 x, y;

    public:
        QLabel* qLabel;
        void display();

        QLabelKeys(QLabel* parent = 0);

    protected:
        bool eventFilter(QObject* obj, QEvent* event);
};

#endif
