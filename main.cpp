#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include "main.h"
#include "InGameUI.h"
#include <QtGlobal>
//#include <unistd.h> // only linux...

InGameUI* inGameUI;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
QList<Client*> clients;
QString nickname, peerAddress, myAddress;
bool isFirstToRun = false;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QImageReader::setAllocationLimit(256);
    #endif

    QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
    quint32 allAddressesSize = allAddresses.size();
    if(allAddressesSize > 2)
    {
        qInfo("My IPs:");
        for(quint32 allAddressesIndex = /*0*/2; allAddressesIndex < allAddressesSize; allAddressesIndex++)
        {
            QHostAddress address = allAddresses[allAddressesIndex];
            qInfo() << addressToString(address);            // broadcast global linkLocal loopback multicast siteLocal uniqueLocalUnicast
            // for real users only global addresses seem interesting
            //qInfo() << address.isBroadcast() << address.isGlobal() << address.isLinkLocal() << address.isLoopback() << address.isMulticast() << address.isSiteLocal() << address.isUniqueLocalUnicast();
        }
    }

    inGameUI = new InGameUI();
    // Uncomment this to see to what type of objects various events are sent
    //app.installEventFilter(new DebugEventFilter);

    bool isDefaultServerPortInUse = isTCPPortInUse(DEFAULT_SERVER_PORT);
    if(!isDefaultServerPortInUse) // assume no two servers running at the same time even for development purpose
        isFirstToRun = getBool("First to run", "Are you the first to run for this game?");
    bool runServer = true,//isFirstToRun/*should be true for more than 2 players*/, // but not required for last player joining the party
         runClient = !isFirstToRun;
    QString isFirstToRunStr = isFirstToRun ? "true" : "false";
    qInfo() << "isFirstToRun:" << isFirstToRunStr; // clicking on exit button is like choosing no...
    quint16 serverPort = DEFAULT_SERVER_PORT + (isDefaultServerPortInUse ? 1 : 0);
    // proposing by default the first server port opened above the default one
    if(/*runServer*/!isFirstToRun) // why would someone want to customize server port or run two servers at the same time ? making interface or configuration file would be more appropriate - running two servers is equivalent to running two clients so for developing purpose it's needed - only the first to launch isn't that much useful to precise
    {
        while(isTCPPortInUse(serverPort))
        {
            serverPort++;
        }
        /*bool ok = false;
        while(!ok)
        {
            QString serverPortStr = getText("Server port", "Your server port", QString::number(DEFAULT_SERVER_PORT));
            uint port = serverPortStr.toUInt(&ok);
            if(port == 0 || port >= 65535)
                ok = false;
            if(!ok)
                showWarningMessage("Invalid port", "Invalid port number.\nThe port number must be an integer between 1 and 65535 (inclusive).");
            else
                serverPort = (quint16) port;
        }*/
        qInfo("serverPort: %hu", serverPort);
    }
    if(runClient)
    {
        peerAddress = getText("Peer address", "A peer address", isDefaultServerPortInUse ? QString::number(DEFAULT_SERVER_PORT) : "");
        if(isAPositiveInteger(peerAddress))
            peerAddress = "localhost:" + peerAddress;
        qInfo() << "peerAddress:" << peerAddress;
    }
    /*QString languageFile = "among_us_decentralized_fr";
        if(translator.load(languageFile))
            app.installTranslator(&translator);
        else
            qInfo("languageFile couldn't be loaded !");*/
    // QSettings like TravianBlockchained

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
        server = new Server(serverPort);
        qInfo() << "Server started on:" << /*serverSocketToString()*/serverPort;
    }
    if(runClient)
    {
        discoverClient(peerAddress);
        qInfo("Waiting for discovery...");
        sleepWithEvents(TIME_S_ASSUME_DISCOVERED);
        qInfo("Discovered %d peers !", getPeers().size());
        qInfo("Waiting for nicknames...");
        QString nicknamesStr = askAll("nicknames"); // or should more precisely ask all nicknames at each nickname test ? but this assume to wait the maximum ping of someone ?
        qInfo("Received nicknames: %s", nicknamesStr.toStdString().c_str());
        QStringList nicknames = nicknamesStr.split(",");
        quint16 nicknamesSize = nicknames.size();
        // assume everybody using external IPv6 - what a strong assumption that doesn't seem necessary
        for(quint16 nicknamesIndex = 0; nicknamesIndex < nicknamesSize; nicknamesIndex++)
        {
            QString nicknameStr = nicknames[nicknamesIndex];
            QStringList nicknameParts = nicknameStr.split(' '); // warning wouldn't work if space in username
            inGameUI->spawnOtherPlayer(nicknameParts[0], nicknameParts[1]);
        }
    }

    QString nickname;
    nickname = getText("Nickname", "Your nickname"); // should check if the given nickname collides with received ones
    qInfo() << "nickname:" << nickname;
    if(runClient)
        sendToAll("nickname " + nickname);

    inGameUI->initialize(nickname);
    inGameUI->resize(640, 480);
    inGameUI->showMaximized();

    return app.exec();
}
