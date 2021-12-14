#ifndef VENTS_H
#define VENTS_H


#include <QPoint>
#include <QMap>
#include <QPainter>
#include <QtMath>
#include <QVector>
#include <QLabel>
#include <QPair>
#include <QtDebug>
#include "qPlus.h"

#define PIXMAP_SIZE_X 500
#define PIXMAP_SIZE_Y 500

#define ARROW_START_X 290
#define ARROW_START_Y 300
#define ARROW_ANCHOR_X -40
#define ARROW_ANCHOR_Y 30
#define ARROW_OFFSET 100



enum VentsID{
    NULL_VENT, 
    WEAPONS,
    NAV_TOP,
    NAV_BOT,
    SHIELDS,
    EAST_CORRIDOR,
    ADMIN,
    CAFETARIA,
    MED_BAY,
    SECURITY,
    ELECTRICAL,
    LOWER_ENGINE,
    REACTOR_BOT,
    REACTOR_TOP,
    UPPER_ENGINE,
    FINAL_VENT,
    };


<<<<<<< HEAD
enum VentsID VentNear(QPoint position);

bool IsThereAnyVentNear(QPoint position);

QLabel* EnterVent(VentsID vent);

QPoint PosOfVent(VentsID vent);
//QPoint onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent);

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image),
    ExitVent();

#endif
=======
#endif
>>>>>>> ebb2347796532c1d4e0190fd0076867bde82496c
