#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QMediaPlayer>
#include "main.h"
#include "InGameUI.h"
//#include <unistd.h> // only linux...

QMediaPlayer* player;
InGameUI* inGameUI;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
QList<Client*> clients;
QString nickname, peerAddress;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    inGameUI = new InGameUI(/*nickname*/);
    app.installEventFilter(inGameUI);
    bool isFirstToRun = getBool("First to run", "Are you the first to run for this party ?"),
         runServer = isFirstToRun/*should be true for more than 2 players*/,
         runClient = !isFirstToRun;
    QString isFirstToRunStr = isFirstToRun ? "true" : "false";
    qInfo(("isFirstToRun: " + isFirstToRunStr).toStdString().c_str()); // clicking on exit button is like choosing no...
    quint16 serverPort = DEFAULT_SERVER_PORT;
    if(runServer)
    {
        QString serverPortStr = getText("Server port", "Your server port");
        serverPort = serverPortStr.toUInt();
        qInfo(("serverPort: " + serverPortStr).toStdString().c_str());
    }
    if(runClient)
    {
        peerAddress = getText("Peer address", "A peer address");
        qInfo(("peerAddress: " + peerAddress).toStdString().c_str());
    }
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
    // waiting a given time and not waiting all to answers and take majority is better I think (likewise if one doesn't cooperate it's not a problem)
    if(runServer)
    {
        qInfo("Starting server...");
        server = new Server(serverPort);
        qInfo("Server started !");
    }
    if(runClient)
    {
        discoverClient(peerAddress);
        qInfo("Waiting discovery...");
        sleepWithEvents(TIME_S_ASSUME_DISCOVERED);
        QString socketString = socketToString(clients[0]/*.back()*/->socket);
        qInfo(("this one: " + socketString).toStdString().c_str());
        qInfo(("Discovered " + QString::number(/*clients*/getPeers().size()) + " peers !").toStdString().c_str());
        qInfo("Waiting nicknames...");
        QString nicknamesStr = askAll("nicknames"); // or should more precisely ask all nicknames at each nickname test ? but this assume to wait the maximum ping of someone ?
        qInfo(("Received nicknames: " + nicknamesStr).toStdString().c_str());
        //if(nicknamesStr != EMPTY_NETWORK_RESPONSE) // should at least contains the first running peer one...
        QStringList nicknames = nicknamesStr.split(",");
        quint16 nicknamesSize = nicknames.size();
        // assume everybody using external IPv6
        for(quint16 nicknamesIndex = 0; nicknamesIndex < nicknamesSize; nicknamesIndex++)
        {
            QString nicknameStr = nicknames[nicknamesIndex];
            QStringList nicknameParts = nicknameStr.split(' '); // warning wouldn't work if space in username
            inGameUI->spawnOtherPlayer(nicknameParts[0], nicknameParts[1]); // ah oui je vois bien pourquoi faire de la POO ça aide ici dans le nom des variables ça permet de se définir des namespace d'une certaine manière ^^
        }
    }

    QString nickname;
    nickname = getText("Nickname", "Your nickname"); // should check with received one
    qInfo(("nickname: " + nickname).toStdString().c_str());
    if(runClient)
        sendToAll("nickname " + nickname);

    inGameUI->initialize(nickname);
    inGameUI->resize(640, 480);
    inGameUI->showMaximized();

    return app.exec();
}
