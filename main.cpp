#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>

#include "QLabelKeys.h"

QMediaPlayer* player;
QLabelKeys* qLabelKeys;

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
    qLabelKeys = new QLabelKeys;
    qLabelKeys->installEventFilter(qLabelKeys);

    qLabelKeys->showMaximized();

    return app.exec();
}
