#include "GameMap.h"
#include "qPlus.h"
#include <QPixmap>
#include <QPoint>

QPixmap* taskIconPixmap = nullptr;
QPixmap* mapLayoutPixmap = nullptr;
QPixmap* sabotageDoorsPixmap = nullptr;

GameMap::GameMap(InGameUI* ui): ui(ui), currPixmap(nullptr) {
    if(!taskIconPixmap)
        taskIconPixmap = getQPixmap("task.png");
    if(!mapLayoutPixmap)
        mapLayoutPixmap = getQPixmap("mapLayout.png");
    if(!sabotageDoorsPixmap)
        sabotageDoorsPixmap = getQPixmap("sabotage_Doors.png");
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
    /*QPen oldPen = newPainter->pen();
    newPainter->setPen(Qt::white);*/
    const int fontSizePt = 15;
    const int fontSizePx = (int)ptToPx(fontSizePt);
    painter->setFont(QFont("Liberation Sans", fontSizePt));
    for(Room& room : ui->rooms) {
        int labelTop;
        QPoint roomCenterMinimap = toMinimapPoint(room.roomCenter);
        bool showSabotageButton = ui->currPlayer.isImpostor && !ui->currPlayer.isGhost && !room.doors.isEmpty();
        if(!showSabotageButton)
            labelTop = roomCenterMinimap.y() - fontSizePx/2;
        else
            labelTop = roomCenterMinimap.y() - sabotageDoorsPixmap->size().height()/2 - fontSizePx/2;
        QRect textRect(roomCenterMinimap.x(), labelTop, 1, fontSizePt);
        painter->drawText(textRect, Qt::TextDontClip | Qt::AlignCenter, room.roomName);
        if(showSabotageButton)
            drawPixmapCentered(painter, roomCenterMinimap, *sabotageDoorsPixmap);
    }
    //newPainter->setPen(oldPen);
    delete painter;
    setPixmap(*newPixmap);
    if(currPixmap)
        delete currPixmap;
    currPixmap = newPixmap;
}