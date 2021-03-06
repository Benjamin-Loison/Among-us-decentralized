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

QColor originalColors[3] = {QColor(0, 255, 0), QColor(255, 0, 0), QColor(0, 0, 255)};
QColor originalColorsAnim[3] = {QColor(0, 255, 0), QColor(255, 16, 16), QColor(0, 0, 255)};

QString assetsFolderImages = "assets/images/";
QString assetsFolderAudio = "assets/audio/";

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
	filePath = assetsFolderImages + filePath;
    //qInfo() << "filePath:" << filePath;
    return new QPixmap(filePath);
}

QPixmap* getQPixmap(quint16 width, quint16 height)
{
    return new QPixmap(width, height);
}

QImage getQImage(QString filePath)
{
    return QImage(assetsFolderImages + filePath);
}

QPixmap colorImage(QImage tmp, QColor color1, QColor color2, QColor color3)
{
    QColor colors[3] = {color1, color2, color3};

    for(quint16 y = 0; y < tmp.height(); y++)
        for(quint16 x = 0; x < tmp.width(); x++)
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 3; originalColorsIndex++) {
                // could check if we are iterating to change a color otherwise can pass, reversing loop order would optimize likewise
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
QPixmap colorImageAnim(QImage tmp, QColor color1, QColor color2, QColor color3)
{
    QColor colors[3] = {color1, color2, color3};

    for(quint16 y = 0; y < tmp.height(); y++)
        for(quint16 x = 0; x < tmp.width(); x++)
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 3; originalColorsIndex++) {
                int alpha = tmp.pixelColor(x,y).alpha();
                QColor newOriginalColor = originalColorsAnim[originalColorsIndex];
                newOriginalColor.setAlpha(alpha);
                if(tmp.pixelColor(x, y) == newOriginalColor) {
                    QColor newColor = colors[originalColorsIndex];
                    newColor.setAlpha(alpha);
                    tmp.setPixelColor(x, y, newColor);
                }
            }
    return QPixmap::fromImage(tmp);
}

QPixmap colorPixmap(const QPixmap& pixmap, QColor color1, QColor color2, QColor color3) {
    QImage tmp = pixmap.toImage();
    return colorImage(tmp, color1, color2, color3);
}
QPixmap colorPixmapAnim(const QPixmap& pixmap, QColor color1, QColor color2, QColor color3) {
    QImage tmp = pixmap.toImage();
    return colorImageAnim(tmp, color1, color2, color3);
}

QPixmap getDisabledButton(const QPixmap &origButton) {
    QImage tmp = origButton.toImage().copy();
    for(quint16 y = 0; y < tmp.height(); y++)
        for(quint16 x = 0; x < tmp.width(); x++) {
            QColor color = tmp.pixelColor(x,y);
            int alpha = color.alpha();
            int gray = (int)(.8*qGray(color.rgb()));
            tmp.setPixelColor(x, y, QColor(gray, gray, gray, alpha));
        }
    return QPixmap::fromImage(tmp);
}

void playSound(QString soundFile)
{
    if(!soundEffectMap.count(soundFile)) {
        QSoundEffect* effect = new QSoundEffect;
        QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolderAudio + soundFile + ".wav").absoluteFilePath());
        effect->setSource(qUrl);
        soundEffectMap[soundFile] = effect;
    }

    soundEffectMap[soundFile]->play(); // run in a separate thread
}

void MessageBoxCross::closeEvent(/*QCloseEvent* closeEvent*/) {
    exit(0);
}

MessageBoxCross::MessageBoxCross(QWidget *parent) : QMessageBox(parent) {}

bool getBool(QString title, QString label)
{
    // X only works if add "| QMessageBox::Close" but it also add a button... same problem as (but potential answer doesn't solve the problem) https://forum.qt.io/topic/74991/qmessagebox-question-does-not-close-when-i-click-on-x-button-but-works-when-we-have-cancel-button-why-so
    MessageBoxCross msgBox(inGameUI);
    msgBox.setWindowTitle(title);
    msgBox.setText(label);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int res = msgBox.exec();
    return res == QMessageBox::Yes;
	// should manage if user clicked on X (still not the case on 01/20/22)
    /*QMessageBox::StandardButton response = QMessageBox::question(inGameUI, title, label, );
    //qInfo() << "response:" << response;
    return response == QMessageBox::Yes;*/
}

quint8 getQUInt8(QString title, QString context, QStringList options)
{
	bool ok;
	QString res = QInputDialog::getItem(inGameUI, title, context, options, 0, true, &ok);
	if(!ok)
		exit(0);
	return options.indexOf(res);
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

void showMessage(QString title, QString label) {
    QMessageBox mb(QMessageBox::Icon::Information, title, label);
    mb.setTextInteractionFlags(Qt::TextSelectableByMouse);
    mb.exec();
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

// https://stackoverflow.com/questions/42141354/convert-pixel-size-to-point-size-for-fonts-on-multiple-platforms
double ptToPx(double pt) {
    return pt/72*QGuiApplication::primaryScreen()->physicalDotsPerInch();
}

double pxToPt(double px) {
    return px*72/QGuiApplication::primaryScreen()->physicalDotsPerInch();
}
