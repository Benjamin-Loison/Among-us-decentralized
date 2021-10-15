#include <QtGlobal>
#include <QtMath>
#include <QString>
#include <QPixmap>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QUrl>
#include "main.h"

QString assetsFolder = "assets/";

double distance(quint16 x0, quint16 y0, quint16 x1, quint16 y1)
{
    return qSqrt(qPow(x0 - x1, 2) + qPow(y0 - y1, 2));
}

QPixmap* getQPixmap(QString filePath)
{
    return new QPixmap(assetsFolder + filePath);
}

void playSound(QString soundFile)
{
    QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolder + soundFile).absoluteFilePath());

    player->setMedia(qUrl);
    player->setVolume(100);
    player->play(); // run in a separate thread
}
