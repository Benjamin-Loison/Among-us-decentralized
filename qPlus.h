#ifndef QPLUS_H
#define QPLUS_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1);
extern QString assetsFolder;
QPixmap* getQPixmap(QString filePath);
QPixmap* getQPixmap(quint16 width,quint16 height);
void playSound(QString soundFile);
QString getText(QString title, QString label);
QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2);

#endif
