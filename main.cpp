#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>
#include <QFileInfo>

#include "qLabelKeys.h"

QMediaPlayer* player;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    /*QString languageFile = "among_us_decentralized_fr";
        if(translator.load(languageFile))
            app.installTranslator(&translator);
        else
            qInfo("languageFile couldn't be loaded !");*/
    // QSettings like TravianBlockchained
    player = new QMediaPlayer;
    QLabelKeys* qLabel = new QLabelKeys;
    qLabel->installEventFilter(qLabel);

    qLabel->showMaximized();

    return app.exec();
}

void playSound(QString soundFile)
{
    QUrl qUrl = QUrl::fromLocalFile(QFileInfo(assetsFolder + soundFile).absoluteFilePath());

    player->setMedia(qUrl);
    player->setVolume(100);
    player->play(); // run in a separate thread
}
