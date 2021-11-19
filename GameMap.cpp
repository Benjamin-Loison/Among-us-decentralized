#include "GameMap.h"
#include "qPlus.h"
#include <QPixmap>
#include <QPoint>

QPixmap* taskIconPixmap = nullptr;
QPixmap* mapLayoutPixmap = nullptr;

GameMap::GameMap(InGameUI* ui): ui(ui), currPixmap(nullptr) {
    if(!taskIconPixmap)
        taskIconPixmap = getQPixmap("task.png");
    if(!mapLayoutPixmap)
        mapLayoutPixmap = getQPixmap("mapLayout.png");
}

void drawPixmapCentered(QPainter* painter, int x, int y, const QPixmap &pixmap) {
    int nx = x-pixmap.width()/2, ny = y-pixmap.height()/2;
    painter->drawPixmap(nx, ny, pixmap);
}

void drawPixmapCentered(QPainter* painter, const QPoint &pt, const QPixmap &pixmap) {
    drawPixmapCentered(painter, pt.x(), pt.y(), pixmap);
}

QPoint GameMap::toMapLayoutPoint(const QPoint &mapPoint) {
    QPixmap* bgPixmap = ui->getBackgroundPixmap();
    return QPoint(mapPoint.x()*mapLayoutPixmap->width()/bgPixmap->width(), mapPoint.y()*mapLayoutPixmap->height()/bgPixmap->height());
}

void GameMap::redraw() {
    QPixmap* newPixmap = new QPixmap(mapLayoutPixmap->size());
    newPixmap->fill(QColor(0, 0, 0, 0));
    QPainter* painter = new QPainter(newPixmap);
    painter->drawPixmap(0, 0, *mapLayoutPixmap);
    for(Task* task : ui->getTasks())
        if(!task->finished)
            drawPixmapCentered(painter, toMapLayoutPoint(task->location), *taskIconPixmap);
    QPoint playerPos(ui->currPlayer.x, ui->currPlayer.y);
    drawPixmapCentered(painter, toMapLayoutPoint(playerPos), *ui->currPlayer.iconOnMapPixmap);
    delete painter;
    setPixmap(*newPixmap);
    if(currPixmap)
        delete currPixmap;
    currPixmap = newPixmap;
}