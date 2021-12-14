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
#include <QMouseEvent>
#include "main.h"
#include "qPlus.h"

#define PIXMAP_SIZE_X 500
#define PIXMAP_SIZE_Y 500

#define ARROW_START_X 290
#define ARROW_START_Y 300
#define ARROW_ANCHOR_X -40
#define ARROW_ANCHOR_Y 30
#define ARROW_OFFSET 70

#define MOUSE_GRID_X 257
#define MOUSE_GRID_Y 338



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


enum VentsID VentNear(QPoint position),
    onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent);

bool IsThereAnyVentNear(QPoint position);

QLabel* EnterVent(VentsID vent);

QPoint PosOfVent(VentsID vent);

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image),
    ExitVent();

#endif
