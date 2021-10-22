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
InGameUI* inGameUI;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
Client* client;
QString nickname, peerAddress;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    inGameUI = new InGameUI;
    
    nickname = getText("Nickname", "Your nickname");
    peerAddress = getText("Peer address", "A peer address");
    
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

    inGameUI->initialize();
    inGameUI->installEventFilter(inGameUI);
    inGameUI->setMinimumSize(640, 480);
    inGameUI->showMaximized();

    return app.exec();
}
