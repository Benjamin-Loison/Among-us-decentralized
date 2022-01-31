#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <QLabel>
#include <QPixmap>
#include "../tasks/Task.h"
#include "InGameUI.h"
#include "../map/Player.h"

class InGameUI;

class GameMap: public QLabel {
    Q_OBJECT
private:
    InGameUI* ui;
    QPixmap* currPixmap;
    QPoint toMinimapPoint(const QPoint &mapPoint);

public:
    GameMap(InGameUI* ui, Map map);
    void redraw();
    void onLeftOrDoubleClick(QMouseEvent *event);
};

#endif
