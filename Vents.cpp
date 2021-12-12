#include "Vents.h"
#include <QMap>

const int VENT_RANGE_SQUARED = qPow(200,2);

QMap<VentsName, QPoint> VentsPositions{
    {WEAPONS, QPoint(6750,690)},
    {NAV_TOP, QPoint(7870,1850)},
    {NAV_BOT, QPoint(7870,2420)},
    {SHIELDS, QPoint(7700,3860)},
    {EAST_CORRIDOR, QPoint(6680,2440)},
    {ADMIN, QPoint(5450,3070)},
    {CAFETARIA, QPoint(5750,1320)},
    {MED_BAY, QPoint(3080,2030)},
    {SECURITY, QPoint(2720,2530)},
    {ELECTRICAL, QPoint(3230,2720)},
    {LOWER_ENGINE, QPoint(2250,3730)},
    {REACTOR_BOT, QPoint(1240,2530)},
    {REACTOR_TOP, QPoint(1050,1820)},
    {UPPER_ENGINE,  QPoint(2230,820)},
    };

QMap<VentsName, QList<VentsName>> VentsLink{
    {WEAPONS, {NAV_TOP}},
    {NAV_TOP, {WEAPONS}},
    {NAV_BOT, {SHIELDS}},
    {SHIELDS, {SHIELDS}},
    {EAST_CORRIDOR, {CAFETARIA,ADMIN}},
    {ADMIN, {CAFETARIA,EAST_CORRIDOR}},
    {CAFETARIA, {EAST_CORRIDOR,ADMIN}},
    {MED_BAY, {ELECTRICAL,SECURITY}},
    {SECURITY, {ELECTRICAL,MED_BAY}},
    {ELECTRICAL, {MED_BAY,SECURITY}},
    {LOWER_ENGINE, {REACTOR_BOT}},
    {REACTOR_BOT, {LOWER_ENGINE}},
    {REACTOR_TOP, {UPPER_ENGINE}},
    {UPPER_ENGINE, {REACTOR_TOP}},
    };



VentsName VentNear(QPoint curr_pos){
    VentsName near_vent = NULL_VENT;
    for (VentsName vent = NULL_VENT; vent < FINAL_VENT; vent++){
        QPoint vent_pos = VentsPosition.value(vent);
        int dist = qPow(curr_pos.y()-vent_pos.y(),2) +  qPow(curr_pos.x()-vent_pos.x(),2)
        if (dist < VENT_RANGE_SQUARED){
            near_vent = vent;
        };
    }
    return near_vent;
}


void drawArrow(QPainter* painter, QPoint pos1,QPoint pos2,QImage arrow){
    qreal angle = qAtan2(pos2.y() - pos1.y(), pos2.x() - pos1.x());
    painter->save();
    painter->rotate(-angle);
    painter->drawImage(ARROW_OFFSET* qCos(angle), ARROW_OFFSET * qCos(angle),arrow);
    painter->restore();
}

