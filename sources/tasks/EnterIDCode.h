#ifndef ENTERIDCODE_H
#define ENTERIDCODE_H

#include <QLabel>
#include <QMouseEvent>

#define DIGIT_NUMBER 6
#define MAX_DIGIT 9
#define DIGIT_UNDEFINED MAX_DIGIT + 1
#define CANCEL DIGIT_UNDEFINED
#define VALIDATE DIGIT_UNDEFINED + 1

#define X_TOP_LEFT_1 279
#define Y_TOP_LEFT_1 68
#define X_LENGTH 66
#define Y_LENGTH 66
#define X_SPACE 18
#define Y_SPACE 12

#define WRITE_X 285
#define WRITE_Y 450
#define WRITE_JUMP 10

extern quint8 code[DIGIT_NUMBER];
extern quint8 answer[DIGIT_NUMBER];
QPair<QPixmap*, QPainter*> getEnterIDCodePixmapPainter();
QLabel* getEnterIDCode();

void onMouseEventEnterIDCode(QMouseEvent* mouseEvent),
    drawAnswer(QPainter* painter),
    onCloseEnterIDCode(),
    resetCode();

#endif
