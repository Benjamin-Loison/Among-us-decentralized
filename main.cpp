#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>
#include "main.h"
#include "QLabelKeys.h"
#include "Server.h"
#include "Client.h"

QMediaPlayer* player;
QLabelKeys* qLabelKeys;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
Client* client;

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

    bool runServer = false,
         runClient = true;
    if(runServer)
    {
        server = new Server;
    }
    if(runClient)
    {
        client = new Client;
    }

    qLabelKeys = new QLabelKeys;
    qLabelKeys->installEventFilter(qLabelKeys);

    qLabelKeys->showMaximized();

    return app.exec();
}
