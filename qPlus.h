#ifndef QPLUS_H
#define QPLUS_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>
#include <QHBoxLayout>
#include <QMessageBox>

class DebugEventFilter : public QObject {
protected:
    bool eventFilter(QObject* obj, QEvent* event);

};

class MessageBoxCross : public QMessageBox {
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

};

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1);
extern QString assetsFolder;
QPixmap* getQPixmap(QString filePath);
QPixmap* getQPixmap(quint16 width,quint16 height);
QPixmap getDisabledButton(const QPixmap &origButton);
void playSound(QString soundFile);
void msleepWithEvents(quint32 ms);
void sleepWithEvents(quint16 s);
QString getText(QString title, QString label, QString defaultText = "");
void showWarningMessage(QString title, QString label);
QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2);
bool getBool(QString title, QString label);
QString randomHex(quint16 length);
QString SHA512(QString input);
QHBoxLayout* makeCenteredLayout(QWidget* widget);
bool isAPositiveInteger(QString s);
QString firstUppercase(QString s);
bool isTCPPortInUse(quint16 port);
void drawCenteredText(QPainter* painter, quint16 x, quint16 y, QString text);
double ptToPx(double pt);
double pxToPt(double px);

#endif
