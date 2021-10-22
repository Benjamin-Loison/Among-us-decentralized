#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <QLabel>
#include <QMouseEvent>

QPair<QPixmap*, QPainter*> getAsterdoisPixmapPainter();
QLabel* getAsteroids();
void fillAsteroids(QPainter* painter, quint8 start, quint16 y, quint16 x),
     onMouseEventAsteroids(QMouseEvent* mouseEvent);

#endif
