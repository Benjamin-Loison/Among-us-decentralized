#include "Vents.h"
#include "../main.h"

const int VENT_RANGE_SQUARED = qPow(150,2);
const int CLICKABLE_RADIUS  = qPow(30,2);

QLabel* ventLabel = nullptr;
QPixmap* ventPixmap = nullptr;
QPixmap* ventArrow = nullptr;

QVector<VentsID> ventsNames{
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

QMap<VentsID, QPoint> ventsPositions{
    {WEAPONS, QPoint(6560,690)},
    {NAV_TOP, QPoint(7870,1850)},
    {NAV_BOT, QPoint(7870,2420)},
    {SHIELDS, QPoint(6700,3860)},
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

QMap<VentsID, QVector<VentsID>> ventsLink{
    {WEAPONS, {NAV_TOP}},
    {NAV_TOP, {WEAPONS}},
    {NAV_BOT, {SHIELDS}},
    {SHIELDS, {NAV_BOT}},
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

QPoint posOfVent(VentsID vent) { return ventsPositions.value(vent);};

VentsID ventNear(QPoint pos){
    VentsID near_vent = NULL_VENT;
    for (VentsID vent : ventsNames){
        QPoint vent_pos = ventsPositions.value(vent);
        int dist = qPow(pos.y()-vent_pos.y(),2) +  qPow(pos.x()-vent_pos.x(),2);
        if (dist < VENT_RANGE_SQUARED)
            near_vent = vent;
    }
    return near_vent;
}

bool isThereAnyVentNear(){
    return (ventNear(QPoint(inGameUI->currPlayer.x, inGameUI->currPlayer.y)) != NULL_VENT) && inGameUI->currPlayer.isImpostor;
}

qreal getAngle(VentsID vent1, VentsID vent2 ){
    QPoint pos1 = ventsPositions.value(vent1);
    QPoint pos2 = ventsPositions.value(vent2);
    qreal angle = (qAtan2(pos2.y()-pos1.y(),pos2.x()-pos1.x())) * (45/qAtan(1));
    return angle;
}

void drawArrow(QPainter* painter, qint16 angle, QImage image){
    painter->save();    
    painter->translate(ARROW_VENT_START_X,ARROW_VENT_START_Y);
    painter->translate(ARROW_VENT_ANCHOR_X,ARROW_VENT_ANCHOR_Y);
    painter->rotate(angle);
    painter->translate(-ARROW_VENT_ANCHOR_X,-ARROW_VENT_ANCHOR_Y);
    painter->drawImage(0,0,image);
    painter->restore();
}

QPair<QPixmap *, QPainter*> getVentPixmapPainter(){
    QPixmap* pixmap = new QPixmap(PIXMAP_SIZE_X,PIXMAP_SIZE_Y);
    pixmap->fill(Qt::transparent);
    QPainter* painter = new QPainter(pixmap);
    return qMakePair(pixmap, painter);
}

QLabel* enterVent(VentsID vent){
    inGameUI->currPlayer.isInvisible = true;
    sendToAll("Vent enter");
    playSound("Vent_open");
    QLabel* qLabel = new QLabel;

    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    
    if(!ventArrow) ventArrow = getQPixmap("Arrow.png");

    QVector<VentsID> linkedvents = ventsLink.value(vent);
    for (VentsID linkedvent : linkedvents){
        qreal angle = getAngle(vent, linkedvent);
        drawArrow(painter, angle, ventArrow->toImage());
    }

    delete painter;
    qLabel->setPixmap(*pixmap);

    if(ventPixmap) {
        delete ventPixmap;
        ventPixmap = nullptr;
    }
    ventPixmap = pixmap;

    if(ventLabel) {
        delete ventLabel;
        ventLabel = nullptr;
    }
    ventLabel = qLabel;

    return qLabel;
}

void exitVent(){
    inGameUI->currPlayer.isInvisible = false;
    inGameUI->currPlayer.sendPosition();
    sendToAll("Vent exit");
    playSound("Vent_open");
    if(ventLabel)
        ventLabel = nullptr;
    if(ventPixmap)
        ventPixmap = nullptr;
}

//very lazy way to check if the arrow is clicked,we just make sure its in a circle with same center as the arrow
bool arrowClicked(QPoint pos, QPoint mouse, qreal angle){
    qreal theta = angle * qAtan(1) / 45;
    QPoint arrow_center = QPoint(pos.x()+ ARROW_VENT_OFFSET * qCos(theta), pos.y()+ ARROW_VENT_OFFSET * qSin(theta));
    return ((qPow(mouse.x() - arrow_center.x(),2)  + qPow(mouse.y() - arrow_center.y(),2))< CLICKABLE_RADIUS);
}

VentsID onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent){
    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter() ;
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    QPoint mouse = QPoint(mouseX,mouseY);

    VentsID new_vent = NULL_VENT;
    QVector<VentsID> linkedvents = ventsLink.value(vent);
    for (VentsID linkedvent : linkedvents){
        qreal angle = getAngle(vent, linkedvent);
        if (arrowClicked(QPoint(MOUSE_GRID_X,MOUSE_GRID_Y),mouse,angle))
            new_vent = linkedvent;
    }
    return new_vent;
}
