#ifndef GAME_MAP_H
#define GAME_MAP_H
#include <QLabel>
#include <QPixmap>
#include "Task.h"
#include "InGameUI.h"

class InGameUI;

class GameMap: public QLabel {
    Q_OBJECT
private:
    InGameUI* ui;
    QPixmap* currPixmap;
    QPoint toMinimapPoint(const QPoint &mapPoint);

public:
    GameMap(InGameUI* ui);
    void redraw();
    void onLeftOrDoubleClick(QMouseEvent *event);
};

#endif