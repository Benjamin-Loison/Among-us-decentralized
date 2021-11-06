#include <QtGlobal>
#include <QtMath>
#include <QString>
#include <QPixmap>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include <QInputDialog>
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


QPixmap* getQPixmap(quint16 width,quint16 height)
{
    return new QPixmap(width,height);
}

QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2) {
    QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)};
    QColor colors[2] = {color1, color2};
    QImage tmp = pixmap.toImage();

    for(quint16 y = 0; y < tmp.height(); y++)
        for(quint16 x = 0; x < tmp.width(); x++)
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 2; originalColorsIndex++)
                if(tmp.pixelColor(x, y) == originalColors[originalColorsIndex])
                    tmp.setPixelColor(x, y, colors[originalColorsIndex]);
    return QPixmap::fromImage(tmp);
}

void playSound(QString soundFile)
{
    QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolder + soundFile).absoluteFilePath());

    player->setMedia(qUrl);
    player->setVolume(100);
    player->play(); // run in a separate thread
}

QString getText(QString title, QString label)
{
    bool ok = false;
    QString text = "";

    while(!ok || text.isEmpty()) // could also check that this username isn't used
    {
        text = QInputDialog::getText(inGameUI, title, label, QLineEdit::Normal, QString(), &ok);
    }
    return text;
}
