#ifndef QPLUS_H
#define QPLUS_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1);
extern QString assetsFolder;
QPixmap* getQPixmap(QString filePath);
void playSound(QString soundFile);

#endif
