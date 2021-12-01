#include <QtGlobal>
#include <QtMath>
#include <QString>
#include <QPixmap>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include <QInputDialog>
#include <QSoundEffect>
#include <QVBoxLayout>
#include "main.h"

QString assetsFolder = "assets/";

QMap<QString, QSoundEffect*> soundEffectMap;
QChar hexs[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

bool DebugEventFilter::eventFilter(QObject *obj, QEvent *ev) {
    if(ev->type() == QEvent::KeyPress)
        qDebug() << "Key press sent to object of type" << obj->metaObject()->className();
    else if(ev->type() == QEvent::KeyRelease)
        qDebug() << "Key release sent to object of type" << obj->metaObject()->className();
    else if(ev->type() == QEvent::MouseMove)
        qDebug() << "Mouse move sent to object of type" << obj->metaObject()->className();
    else if(ev->type() == QEvent::MouseButtonPress)
        qDebug() << "Mouse button press sent to object of type" << obj->metaObject()->className();
    else if(ev->type() == QEvent::MouseButtonDblClick)
        qDebug() << "Mouse double click sent to object of type" << obj->metaObject()->className();
    return false;
}

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
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 2; originalColorsIndex++) {
                int alpha = tmp.pixelColor(x,y).alpha();
                QColor newOriginalColor = originalColors[originalColorsIndex];
                newOriginalColor.setAlpha(alpha);
                if(tmp.pixelColor(x, y) == newOriginalColor) {
                    QColor newColor = colors[originalColorsIndex];
                    newColor.setAlpha(alpha);
                    tmp.setPixelColor(x, y, newColor);
                }
            }
    return QPixmap::fromImage(tmp);
}

void playSound(QString soundFile)
{
    if(!soundEffectMap.count(soundFile)) {
        QSoundEffect* effect = new QSoundEffect;
        QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolder + soundFile).absoluteFilePath());
        effect->setSource(qUrl);
        soundEffectMap[soundFile] = effect;
    }

    soundEffectMap[soundFile]->play(); // run in a separate thread
}

bool getBool(QString title, QString label)
{
    int reponse = QMessageBox::question(inGameUI, title, label, QMessageBox ::Yes | QMessageBox::No);
    return reponse == QMessageBox::Yes;
}

QString getText(QString title, QString label, QString defaultText)
{
    bool ok = false;
    QString text = "";

    while(!ok || text.isEmpty()) // could also check that this username isn't used
    {
        text = QInputDialog::getText(inGameUI, title, label, QLineEdit::Normal, defaultText, &ok);
    }
    return text;
}

void showWarningMessage(QString title, QString label) {
    QMessageBox(QMessageBox::Icon::Warning, title, label).exec();
}

void msleepWithEvents(quint32 ms)
{
    qint64 endTime = QDateTime::currentMSecsSinceEpoch() + ms;
    while(QDateTime::currentMSecsSinceEpoch() < endTime)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }
}

void sleepWithEvents(quint16 s)
{
    msleepWithEvents(s * 1000);
}

QString randomHex(quint16 length)
{
    QString res = "";
    for(quint16 i = 0; i < length / 4; i++)
    {
        quint32 randomNb = QRandomGenerator::global()->bounded(16);
        QChar hex = hexs[randomNb];
        res.append(hex);
    }
    return res;
}

QString SHA512(QString input)
{
    // ifdef depending on Qt version seems the best way to make it compatible
    QCryptographicHash cryptographicHash(QCryptographicHash::Sha512)/* = QCryptographicHash(QCryptographicHash::Sha512)*/; // can't do classical " = QCryptographicHash" on Linux... (Qt < 6.1.1) (use of deleted function QCryptographicHash::QCryptographicHash(const QCryptographicHash&) with Qt 5.12.8)
    QByteArray byteArray = input.toUtf8(),
               hashedByteArray = cryptographicHash.hash(byteArray, QCryptographicHash::Sha512); // why have to repeat it ?!
    QString hashed = hashedByteArray.toHex();
    return hashed;
}

QHBoxLayout* makeCenteredLayout(QWidget* widget) {
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(widget);
    vLayout->addStretch();
    QHBoxLayout* ret = new QHBoxLayout;
    ret->addStretch();
    ret->addLayout(vLayout);
    ret->addStretch();
    return ret;
}

bool isAPositiveInteger(QString s)
{
    quint8 sLength = s.length();
    for(quint8 sIndex = 0; sIndex < sLength; sIndex++)
    {
        if(!s[sIndex].isDigit())
            return false;
    }
    return true;
}
