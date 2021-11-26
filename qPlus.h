#ifndef QPLUS_H
#define QPLUS_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>

class DebugEventFilter : public QObject {
protected:
    bool eventFilter(QObject* obj, QEvent* event);

};

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1);
extern QString assetsFolder;
QPixmap* getQPixmap(QString filePath);
QPixmap* getQPixmap(quint16 width,quint16 height);
void playSound(QString soundFile);
void msleepWithEvents(quint32 ms);
void sleepWithEvents(quint16 s);
QString getText(QString title, QString label, QString defaultText = "");
void showWarningMessage(QString title, QString label);
QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2);
bool getBool(QString title, QString label);
QString randomHex(quint16 length);
QString SHA512(QString input);

#endif
