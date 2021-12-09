#ifndef ALIGNENGINE_H
#define ALIGNENGINE_H

#include <QLabel>
#include <QMouseEvent>



QPair<QPixmap*, QPainter*> getAlignEnginePixmapPainter();
QLabel* getAlignEngine();

void onCloseAlignEngine();

#endif