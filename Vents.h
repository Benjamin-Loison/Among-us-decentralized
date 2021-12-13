#ifndef VENTS_H
#define VENTS_H


#include <Qpoint>
#include <QMap>
#include <QPainter>
#include <QtMath>
#include <QVector>


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

enum VentsID VentNear(QPoint curr_pos);

#endif