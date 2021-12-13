#include "Vents.h"

const int VENT_RANGE_SQUARED = qPow(200,2);
const int CLICKABLE_RADIUS  = qPow(100,2);


QVector<VentsID> VentsNames{
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

QMap<VentsID, QPoint> VentsPositions{
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

QMap<VentsID, QVector<VentsID>> VentsLink{
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






enum VentsID VentNear(QPoint curr_pos){
    enum VentsID near_vent = NULL_VENT;
    for (int i = 0; i< VentsNames.size(); i++){
        enum VentsID vent = VentsNames.at(i);
        QPoint vent_pos = VentsPositions.value(vent);
        int dist = qPow(curr_pos.y()-vent_pos.y(),2) +  qPow(curr_pos.x()-vent_pos.x(),2);
        if (dist < VENT_RANGE_SQUARED){
            near_vent = vent;
        };
    }
    return near_vent;
}



/*
qint16 GetAngle(VentsName vent1, VentsName vent2){
    QPoint pos1 = VentsPositions.value(vent1);
    QPoint pos2 = VentsPositions.value(vent2);
    qint16 angle = qAtan2(pos2.y()-pos1.y(),pos2.x()-pos1.x());
    return angle;
}

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image){
    painter->save();
    painter->translate(pos);
    painter->rotate(-angle);
    painter->drawImage(ARROW_OFFSET * qCos(angle), ARROW_OFFSET * qSin(angle),image);
    painter->restore();
}

//very lazy way to check if the arrow is clicked,we just make sure its in a circle with same center as the arrow
bool ArrowClicked(QPoint pos, QPoint mouse, qint16 angle){
    QPoint arrow_center = QPoint(pos.x()+ ARROW_OFFSET * qCos(angle), pos.y()+ ARROW_OFFSET * qSin(angle));
    return ((qPow(mouse.x(),2)  + qPow(mouse.y(),2))< CLICKABLE_RADIUS);
}

*/
