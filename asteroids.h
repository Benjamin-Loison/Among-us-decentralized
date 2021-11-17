#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <QLabel>
#include <QMouseEvent>

struct Asteroid {
    int x, y;
    double vx, vy; // velocity in pixels per second
    QPixmap* pixmap;
    bool destroyed = false;

    Asteroid();
    Asteroid(int x, int y, double unit_x, double unit_y, QPixmap* pixmap);
};

QPair<QPixmap*, QPainter*> getAsteroidsPixmapPainter();
QLabel* getAsteroids(qint64 now, QObject* filterObj);
void fillAsteroids(QPainter* painter, quint8 start, quint16 y, quint16 x),
     onMouseEventAsteroids(QMouseEvent* mouseEvent),
     redrawAsteroids(qint64 now),
     resetAsteroids(),
     onCloseAsteroids();

#endif
