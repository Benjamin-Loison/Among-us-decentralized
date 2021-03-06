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
#include "../main.h"
#include "../qPlus.h"

#define PIXMAP_SIZE_X 500
#define PIXMAP_SIZE_Y 500

#define ARROW_VENT_START_X 290
#define ARROW_VENT_START_Y 300
#define ARROW_VENT_ANCHOR_X -40
#define ARROW_VENT_ANCHOR_Y 30
#define ARROW_VENT_OFFSET 80

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

VentsID ventNear(QPoint position),
        onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent);

bool isThereAnyVentNear();

QLabel* enterVent(VentsID vent);

QPoint posOfVent(VentsID vent);

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image),
     exitVent();

#endif
