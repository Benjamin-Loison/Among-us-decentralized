#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>
#include "main.h"
#include "InGameUI.h"
#include "Server.h"
#include "Client.h"

QMediaPlayer* player;
InGameUI* qLabelKeys;
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

    qLabelKeys = new InGameUI;
    qLabelKeys->installEventFilter(qLabelKeys);
    qLabelKeys->setMinimumSize(640, 480);
    qLabelKeys->showMaximized();

    return app.exec();
}
