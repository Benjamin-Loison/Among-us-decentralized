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
using namespace std;

QString assetsFolder = "assets/";

QMap<QString, QSoundEffect*> soundEffectMap;
QChar hexs[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

bool DebugEventFilter::eventFilter(QObject* obj, QEvent* ev)
{
    QString source = "";
    switch(ev->type())
    {
        case QEvent::KeyPress:
            source = "Key press";
            break;
        case QEvent::KeyRelease:
            source = "Key release";
            break;
        case QEvent::MouseMove:
            source = "Mouse move";
            break;
        case QEvent::MouseButtonPress:
            source = "Mouse button press";
            break;
        case QEvent::MouseButtonDblClick:
            source = "Mouse double click";
        default:
            ;
    }
    if(source != "")
        qDebug() << source << "sent to object of type" << obj->metaObject()->className();
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

QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)};

QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2) {
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

void playSound(QString soundFile) // could assume all sounds are .wav so can append it here
{
    if(!soundEffectMap.count(soundFile)) {
        QSoundEffect* effect = new QSoundEffect;
        QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolder + soundFile).absoluteFilePath());
        effect->setSource(qUrl);
        soundEffectMap[soundFile] = effect;
    }

    soundEffectMap[soundFile]->play(); // run in a separate thread
}

void MessageBoxCross::closeEvent(QCloseEvent* closeEvent) {
    exit(0);
}

bool getBool(QString title, QString label)
{
    // X only works if add "| QMessageBox::Close" but it also add a button... same problem as (but potential answer doesn't solve the problem) https://forum.qt.io/topic/74991/qmessagebox-question-does-not-close-when-i-click-on-x-button-but-works-when-we-have-cancel-button-why-so
    MessageBoxCross msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(label);
    msgBox.setStandardButtons(QMessageBox ::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    return ret == QMessageBox::Yes;
    /*QMessageBox::StandardButton response = QMessageBox::question(inGameUI, title, label, );
    //qInfo() << "response:" << response;
    return response == QMessageBox::Yes;*/
}

QString getText(QString title, QString label, QString defaultText)
{
    bool ok = false;
    QString text = "";
    while(text == "") {
        text = QInputDialog::getText(inGameUI, title, label, QLineEdit::Normal, defaultText, &ok);
        if(!ok)
            exit(0);
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

QString randomHex(quint16 length) // length in bits
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
    return all_of(s.begin(), s.end(), [](QChar c) { return c.isDigit(); });
}

QString firstUppercase(QString s)
{
    if(s == "") return s;
    s[0] = s[0].toTitleCase();
    return s;
}

// doesn't seem to always work perfectly if quick restarting
bool isTCPPortInUse(quint16 port)
{
    QTcpSocket* socket = new QTcpSocket();
    bool res = socket->bind(port, QAbstractSocket::DontShareAddress | QAbstractSocket::ReuseAddressHint);
    delete(socket);
    return !res;
}

void drawCenteredText(QPainter* painter, quint16 x, quint16 y, QString text)
{
    QFontMetrics fm(painter->font());
    QRect rect = fm.boundingRect(text);
    painter->drawText(x - rect.width() / 2, y + rect.height() / 2, text);
}
