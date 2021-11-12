#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>
#include "main.h"
#include "InGameUI.h"
#include <unistd.h> // only linux...

QMediaPlayer* player;
InGameUI* inGameUI;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
QList<Client*> clients;
QString nickname, peerAddress;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    bool isFirstToRun = true,
         runServer = isFirstToRun,
         runClient = !isFirstToRun;

    quint16 serverPort = DEFAULT_SERVER_PORT;
    if(runServer)
        serverPort = getText("Server port", "Your server port").toUInt();
    if(runClient)
        peerAddress = getText("Peer address", "A peer address");
    inGameUI = new InGameUI(nickname);
    /*QString languageFile = "among_us_decentralized_fr";
        if(translator.load(languageFile))
            app.installTranslator(&translator);
        else
            qInfo("languageFile couldn't be loaded !");*/
    // QSettings like TravianBlockchained
    player = new QMediaPlayer;

    // les nouveaux se connectent aux anciens
    // disons que l'on cherche à découvrir tout le monde (dans le cas précis de ce jeu)
    // j'ai préféré l'approche on attend un certain temps fixé que attendre un certain nombre fixé car quelqu'un pourrait faire croire que certains participants (qu'il a généré) représentent les autres
    // disons que l'on exécute un message contenant de l'information seulement lorsque la majorité l'approuve (dans le cas précis de ce jeu sinon résultat d'impossibilité de FLP)
    // on fait l'hypothèse que les gens laissent du temps s'écouler entre les actions réseaux lorsque cela est nécessaire
    // ou par rapport à la ligne précédente, faisons un modèle où chaque participant envoie aux autres juste ses actions (donc confiance entière envers les autres)
    // exemple pratique: jeu où les joueurs participent avec des arbitres vérifiant que personne ne triche
    if(runServer)
    {
        server = new Server(serverPort);
    }
    if(runClient)
    {
        discoverClient(peerAddress);
        qInfo(socketToString(clients.back()->socket).toStdString().c_str());
        sleep(TIME_S_ASSUME_DISCOVERED);
        askAll("nicknames");
    }
    nickname = getText("Nickname", "Your nickname"); // should check with received one

    inGameUI->initialize();
    app.installEventFilter(inGameUI);
    inGameUI->resize(640, 480);
    inGameUI->showMaximized();

    return app.exec();
}
