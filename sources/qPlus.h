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
    void closeEvent(/*QCloseEvent *event*/);

public:
    MessageBoxCross(QWidget *parent = nullptr);
};

extern QColor originalColors[3];

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1);
extern QString assetsFolderImages;
extern QString assetsFolderAudio;
QPixmap* getQPixmap(QString filePath);
QPixmap* getQPixmap(quint16 width, quint16 height);
QImage getQImage(QString filePath);
QPixmap getDisabledButton(const QPixmap &origButton);
void playSound(QString soundFile);
void msleepWithEvents(quint32 ms);
void sleepWithEvents(quint16 s);
QString getText(QString title, QString label, QString defaultText = "");
void showWarningMessage(QString title, QString label);
void showMessage(QString title, QString label);
QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2, QColor color3 = originalColors[2]);
QPixmap colorImage(QImage tmp, QColor color1, QColor color2, QColor color3 = originalColors[2]);
QPixmap colorPixmapAnim(const QPixmap& pixmap, QColor color1, QColor color2, QColor color3 = originalColors[2]);
QPixmap colorImageAnim(QImage tmp, QColor color1, QColor color2, QColor color3 = originalColors[2]);
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
quint8 getQUInt8(QString title, QString context, QStringList options);

#endif
