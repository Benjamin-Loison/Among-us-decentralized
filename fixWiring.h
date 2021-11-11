#ifndef FIXWIRING_H
#define FIXWIRING_H

#include <QLabel>
#include <QMouseEvent>

#define FIX_WIRING_LEFT_X 4
#define FIX_WIRING_RIGHT_X 470
#define FIX_WIRING_WIDTH 31
#define FIX_WIRING_HEIGHT 17
#define FIX_WIRING_DELTA_Y 103
#define FIX_WIRING_TOP_Y 96

#define COLORS_NUMBER 4
#define COLOR_UNDEFINED COLORS_NUMBER + 1
#define COLOR_FIXING COLORS_NUMBER + 2

enum WiringColor{Yellow, Blue, Red, Magenta};
QPair<QPixmap*, QPainter*> getFixWiringPixmapPainter();
QLabel* getFixWiring();
extern quint8 links[COLORS_NUMBER];
quint16 getYForWiring(quint8 nodesIndex);
quint8 getIndex(WiringColor color);
void fillWire(QPainter* painter, quint8 start),
     fillFixWire(QPainter* painter, quint8 start, quint16 y, quint16 x),
     onMouseEventFixWiring(QMouseEvent* mouseEvent),
     onCloseFixWiring(),
     resetFixWiring();
extern WiringColor lefts[COLORS_NUMBER],
                   rights[COLORS_NUMBER];

#endif
