#include "GameMap.h"
#include "qPlus.h"
#include "Server.h"
#include <QPixmap>
#include <QPoint>

QPixmap* taskIconPixmap = nullptr;
QPixmap* mapLayoutPixmap = nullptr;
QPixmap* sabotageDoorsPixmap = nullptr;
QPixmap* disabledSabotageDoorsPixmap;
//QPoint lastClick;

GameMap::GameMap(InGameUI* ui): ui(ui), currPixmap(nullptr) {
    if(!taskIconPixmap)
        taskIconPixmap = getQPixmap("task.png");
    if(!mapLayoutPixmap)
        mapLayoutPixmap = getQPixmap("mapLayout.png");
    if(!sabotageDoorsPixmap) {
        sabotageDoorsPixmap = getQPixmap("sabotage_Doors.png");
        disabledSabotageDoorsPixmap = new QPixmap(getDisabledButton(*sabotageDoorsPixmap));
    }
}

QRect getSabotageIconRect(const QPoint &roomCenterMinimap) {
    return QRect(
        roomCenterMinimap.x()-sabotageDoorsPixmap->width()/2,
        roomCenterMinimap.y()-sabotageDoorsPixmap->height()/2,
        sabotageDoorsPixmap->width(),
        sabotageDoorsPixmap->height());
}

void drawPixmapCentered(QPainter* painter, int x, int y, const QPixmap &pixmap) {
    int nx = x-pixmap.width()/2, ny = y-pixmap.height()/2;
    painter->drawPixmap(nx, ny, pixmap);
}

void drawPixmapCentered(QPainter* painter, const QPoint &pt, const QPixmap &pixmap) {
    drawPixmapCentered(painter, pt.x(), pt.y(), pixmap);
}

QPoint GameMap::toMinimapPoint(const QPoint &mapPoint) {
    QPixmap* bgPixmap = ui->getBackgroundPixmap();
    return QPoint(mapPoint.x()*mapLayoutPixmap->width()/bgPixmap->width(), mapPoint.y()*mapLayoutPixmap->height()/bgPixmap->height());
}

void GameMap::redraw() {
    QPixmap* newPixmap = new QPixmap(mapLayoutPixmap->size());
    newPixmap->fill(QColor(0, 0, 0, 0));
    QPainter* painter = new QPainter(newPixmap);
    // Map layout
    painter->drawPixmap(0, 0, *mapLayoutPixmap);

    // Player icon
    QPoint playerPosMinimap = toMinimapPoint(QPoint(ui->currPlayer.x, ui->currPlayer.y));
    QSize iconSize = ui->currPlayer.iconOnMapPixmap->size();
    QPoint topLeftCorner(playerPosMinimap.x() - iconSize.width()/2, playerPosMinimap.y() - iconSize.height());
    painter->drawPixmap(topLeftCorner, *ui->currPlayer.iconOnMapPixmap);

    // Tasks
    for(Task* task : ui->getTasks())
        if(!task->finished)
            drawPixmapCentered(painter, toMinimapPoint(task->location), *taskIconPixmap);

    // Rooms and door sabotage buttons
    const int fontSizePt = 15;
    const int fontSizePx = (int)ptToPx(fontSizePt);
    painter->setFont(QFont("Liberation Sans", fontSizePt));
    for(Room& room : ui->rooms) {
        QPoint roomCenterMinimap = toMinimapPoint(room.roomCenter);
        bool showSabotageButton = ui->currPlayer.isImpostor && !ui->currPlayer.isGhost && !room.doors.isEmpty();
        int labelTop = roomCenterMinimap.y() - (showSabotageButton ? sabotageDoorsPixmap->size().height()/2 : 0) - fontSizePx/2;
        QRect textRect(roomCenterMinimap.x(), labelTop, 1, fontSizePt);
        painter->drawText(textRect, Qt::TextDontClip | Qt::AlignCenter, room.roomName);
        if(showSabotageButton)
            drawPixmapCentered(painter, roomCenterMinimap, *(room.isCoolingDown() ? disabledSabotageDoorsPixmap : sabotageDoorsPixmap));
    }

    // Debugging cross
    /*QPen debugPen(QColor(0, 200, 0));
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    painter->drawLine(lastClick.x()-10, lastClick.y(), lastClick.x()+10, lastClick.y());
    painter->drawLine(lastClick.x(), lastClick.y()-10, lastClick.x(), lastClick.y()+10);*/

    delete painter;
    setPixmap(*newPixmap);
    if(currPixmap)
        delete currPixmap;
    currPixmap = newPixmap;
}

void GameMap::onLeftOrDoubleClick(QMouseEvent *event) {
    qInfo() << "Parent position:" << event->pos() << "- Minimap position:" << mapFromParent(event->pos());
    //lastClick = mapFromParent(event->pos());
    if(ui->currPlayer.isImpostor && !ui->currPlayer.isGhost)
        for(Room &room : ui->rooms)
            if(!room.isCoolingDown() && getSabotageIconRect(toMinimapPoint(room.roomCenter)).contains(mapFromParent(event->pos()))) {
                room.sabotage();
                sendToAll(QString("Sabotage_doors %1").arg(room.id));
                return;
            }
}
