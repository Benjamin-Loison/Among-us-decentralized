<<<<<<< HEAD
#include "Vents.h"

const int VENT_RANGE_SQUARED = qPow(150,2);
const int CLICKABLE_RADIUS  = qPow(20,2);

QLabel* VentLabel = nullptr;
QPixmap* VentPixmap = nullptr;


QPixmap* VentArrow = nullptr;


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

QMap<VentsID, QVector<VentsID>> VentsLink{
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


QPoint PosOfVent(VentsID vent) {VentsPositions.value(vent);};

enum VentsID VentNear(QPoint pos){
    enum VentsID near_vent = NULL_VENT;
    for (int i = 0; i< VentsNames.size(); i++){
        enum VentsID vent = VentsNames.at(i);
        QPoint vent_pos = VentsPositions.value(vent);
        int dist = qPow(pos.y()-vent_pos.y(),2) +  qPow(pos.x()-vent_pos.x(),2);
        if (dist < VENT_RANGE_SQUARED){
            near_vent = vent;
        };
    }
    return near_vent;
}

bool IsThereAnyVentNear(QPoint position){
    return (VentNear(position) != NULL_VENT);
}



qreal GetAngle(VentsID vent1, VentsID vent2 ){
    QPoint pos1 = VentsPositions.value(vent1);
    QPoint pos2 = VentsPositions.value(vent2);
    qreal angle = (qAtan2(pos2.y()-pos1.y(),pos2.x()-pos1.x())) * (45/qAtan(1));
    return angle;
}

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image){
    painter->save();    
    painter->translate(ARROW_START_X,ARROW_START_Y);
    painter->translate(ARROW_ANCHOR_X,ARROW_ANCHOR_Y);
    painter->rotate(angle);
    painter->translate(-ARROW_ANCHOR_X,-ARROW_ANCHOR_Y);
    painter->drawImage(0,0,image);
    painter->restore();
}


QPair<QPixmap *, QPainter*> getVentPixmapPainter(){
    QPixmap* pixmap = new QPixmap(PIXMAP_SIZE_X,PIXMAP_SIZE_Y);
    pixmap->fill(Qt::transparent);
    QPainter* painter = new QPainter(pixmap);
    return qMakePair(pixmap, painter);
}

QLabel* EnterVent(VentsID vent){
    playSound("Vent_open.wav");
    QLabel* qLabel = new QLabel;

    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    
    if(!VentArrow) {VentArrow = getQPixmap("Arrow.png");};

    QVector<VentsID> linkedvents = VentsLink.value(vent); 
    for (int i = 0; i< linkedvents.size(); i++){
        qreal angle = GetAngle(vent, linkedvents.at(i));
        drawArrow(painter, VentsPositions.value(vent), angle, VentArrow->toImage());
    }

    delete painter;
    qLabel->setPixmap(*pixmap);

    if(VentPixmap) {
        delete VentPixmap;
        VentPixmap = nullptr;
    }
    VentPixmap = pixmap;

    if(VentLabel) {
        delete VentLabel;
        VentLabel = nullptr;
    }
    VentLabel = qLabel;

    return qLabel;
}


void ExitVent(){
    playSound("Vent_open.wav");
    if(VentLabel)
        VentLabel = nullptr;
    if(VentPixmap)
        VentPixmap = nullptr;
}


//very lazy way to check if the arrow is clicked,we just make sure its in a circle with same center as the arrow
bool ArrowClicked(QPoint pos, QPoint mouse, qreal angle){
    qreal theta = angle * qAtan(1) / 45;
    QPoint arrow_center = QPoint(pos.x()+ ARROW_OFFSET * qCos(theta), pos.y()+ ARROW_OFFSET * qSin(theta));
    return ((qPow(mouse.x() - arrow_center.x(),2)  + qPow(mouse.y() - arrow_center.y(),2))< CLICKABLE_RADIUS);
}


enum VentsID onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent){
    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter() ;
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    QPoint mouse = QPoint(mouseX,mouseY);

    VentsID new_vent = NULL_VENT;
    QVector<VentsID> linkedvents = VentsLink.value(vent);
    for (int i = 0; i< linkedvents.size(); i++){
        qreal angle = GetAngle(vent, linkedvents.at(i));
        if (ArrowClicked(QPoint(MOUSE_GRID_X,MOUSE_GRID_Y),mouse,angle)){
            new_vent = linkedvents.at(i);
        };
    }
    return new_vent;
}
=======
#include "Vents.h"

const int VENT_RANGE_SQUARED = qPow(150,2);
const int CLICKABLE_RADIUS  = qPow(20,2);

QLabel* VentLabel = nullptr;
QPixmap* VentPixmap = nullptr;
QPixmap* VentArrow = nullptr;

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

QMap<VentsID, QVector<VentsID>> VentsLink{
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

QPoint PosOfVent(VentsID vent) {VentsPositions.value(vent);};

VentsID VentNear(QPoint pos){
    VentsID near_vent = NULL_VENT;
    for (VentsID vent : VentsNames){
        QPoint vent_pos = VentsPositions.value(vent); // doesn't create the key, value pair if the key doesn't exist
        int dist = qPow(pos.y()-vent_pos.y(),2) +  qPow(pos.x()-vent_pos.x(),2);
        if (dist < VENT_RANGE_SQUARED)
            near_vent = vent;
    }
    return near_vent;
}

bool isThereAnyVentNear(){
    return inGameUI->currPlayer.isImpostor && (VentNear(QPoint(inGameUI->currPlayer.x, inGameUI->currPlayer.y)) != NULL_VENT);
}

qreal GetAngle(VentsID vent1, VentsID vent2 ){
    QPoint pos1 = VentsPositions.value(vent1);
    QPoint pos2 = VentsPositions.value(vent2);
    qreal angle = (qAtan2(pos2.y()-pos1.y(),pos2.x()-pos1.x())) * (45/qAtan(1));
    return angle;
}

void drawArrow(QPainter* painter, QPoint pos, qint16 angle, QImage image){
    painter->save();    
    painter->translate(ARROW_START_X,ARROW_START_Y);
    painter->translate(ARROW_ANCHOR_X,ARROW_ANCHOR_Y);
    painter->rotate(angle);
    painter->translate(-ARROW_ANCHOR_X,-ARROW_ANCHOR_Y);
    painter->drawImage(0,0,image);
    painter->restore();
}


QPair<QPixmap *, QPainter*> getVentPixmapPainter(){
    QPixmap* pixmap = new QPixmap(PIXMAP_SIZE_X,PIXMAP_SIZE_Y);
    pixmap->fill(Qt::transparent);
    QPainter* painter = new QPainter(pixmap);
    return qMakePair(pixmap, painter);
}

QLabel* EnterVent(VentsID vent){
    playSound("Vent_open.wav");
    QLabel* qLabel = new QLabel;

    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    
    if(!VentArrow)
        VentArrow = getQPixmap("Arrow.png");

    QVector<VentsID> linkedvents = VentsLink.value(vent); 
    for (VentsID linkedvent : linkedvents){
        qreal angle = GetAngle(vent, linkedvent);
        drawArrow(painter, VentsPositions.value(vent), angle, VentArrow->toImage());
    }

    delete painter;
    qLabel->setPixmap(*pixmap);

    if(VentPixmap) {
        delete VentPixmap;
        VentPixmap = nullptr;
    }
    VentPixmap = pixmap;

    if(VentLabel) {
        delete VentLabel;
        VentLabel = nullptr;
    }
    VentLabel = qLabel;

    return qLabel;
}

void ExitVent(){
    playSound("Vent_open.wav");
    if(VentLabel)
        VentLabel = nullptr;
    if(VentPixmap)
        VentPixmap = nullptr;
}

//very lazy way to check if the arrow is clicked,we just make sure its in a circle with same center as the arrow
bool ArrowClicked(QPoint pos, QPoint mouse, qreal angle){
    qreal theta = angle * qAtan(1) / 45;
    QPoint arrow_center = QPoint(pos.x()+ ARROW_OFFSET * qCos(theta), pos.y()+ ARROW_OFFSET * qSin(theta));
    return ((qPow(mouse.x() - arrow_center.x(),2)  + qPow(mouse.y() - arrow_center.y(),2))< CLICKABLE_RADIUS);
}

enum VentsID onMouseEventVent(VentsID vent, QMouseEvent* mouseEvent){
    QPair<QPixmap*, QPainter*> pixmapPainter = getVentPixmapPainter() ;
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    QPoint mouse = QPoint(mouseX,mouseY);

    VentsID new_vent = NULL_VENT;
    QVector<VentsID> linkedvents = VentsLink.value(vent);
    for (VentsID linkedvent : linkedvents){
        qreal angle = GetAngle(vent, linkedvent);
        if (ArrowClicked(QPoint(MOUSE_GRID_X,MOUSE_GRID_Y),mouse,angle))
            new_vent = linkedvent;
    }
    return new_vent;
}
>>>>>>> 9d2a055cd7498601c773d2a1a27b15490b5cf67a
