#ifndef ALIGNENGINE_H
#define ALIGNENGINE_H

#include <QLabel>
#include <QMouseEvent>

#define PATTERN_X_START 60
#define PATTERN_Y_START 265
#define PATTERN_ANCHOR_X 585
#define PATTERN_ANCHOR_Y 200

#define ARROW_X_START 650
#define ARROW_Y_START 390
#define ARROW_ANCHOR_X 1000
#define ARROW_ANCHOR_Y 76


QPair<QPixmap*, QPainter*> getAlignEnginePixmapPainter();
QLabel* getAlignEngine();

void drawAssets(QPainter* painter, qreal angle),
    onCloseAlignEngine(),
    onMouseEventAlignEngine(QMouseEvent* mouseEvent);

#endif