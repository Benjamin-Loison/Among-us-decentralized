#include <QApplication>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include "main.h"
#include "ui/InGameUI.h"
#include <QtGlobal>

#ifdef __linux__
	#include <signal.h>
#endif

InGameUI* inGameUI;
InGameGUI currentInGameGUI = IN_GAME_GUI_NONE;
Server* server;
QList<Client*> clients;
QString myAddress;
bool isFirstToRun = false;
QTranslator translator;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QImageReader::setAllocationLimit(512);
    #endif

    QString locale = QLocale::system().name().section('_', 0, 0),
            languageFile = "AmongUsDecentralizedFR";
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if(!translator.load(languageFile))
        qInfo("languageFile couldn't be loaded !");
    app.installTranslator(&translator);

    QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
    quint32 allAddressesSize = allAddresses.size();
    if(allAddressesSize > 2) // could check more precisely if not just having an IPv6
    {
        qInfo("My IPs:");
        for(quint32 allAddressesIndex = /*0*/2; allAddressesIndex < allAddressesSize; allAddressesIndex++)
        {
            QHostAddress address = allAddresses[allAddressesIndex];
            if(address.protocol() != QAbstractSocket::IPv6Protocol)
                qInfo() << addressToString(address);
            // for real users only global addresses seem interesting
            // broadcast global linkLocal loopback multicast siteLocal uniqueLocalUnicast
            //qInfo() << address.isBroadcast() << address.isGlobal() << address.isLinkLocal() << address.isLoopback() << address.isMulticast() << address.isSiteLocal() << address.isUniqueLocalUnicast();
        }
    }

    inGameUI = new InGameUI();
    // Uncomment this to see to what type of objects various events are sent
    //app.installEventFilter(new DebugEventFilter);

    bool isDefaultServerPortInUse = isTCPPortInUse(DEFAULT_SERVER_PORT);
    if(!isDefaultServerPortInUse) // assume no two servers running at the same time even for development purpose
        isFirstToRun = getBool(QObject::tr("First to run"), QObject::tr("Are you the first to run for this game ?"));
    bool runServer = true, // not required for last player joining the party - could always open it, it doesn't cost a lot and if someone want to use this node it is possible
         runClient = !isFirstToRun;
    QString isFirstToRunStr = isFirstToRun ? "true" : "false";
    qInfo() << "isFirstToRun:" << isFirstToRunStr; // clicking on exit button is like choosing no...
    quint16 serverPort = DEFAULT_SERVER_PORT + (isDefaultServerPortInUse ? 1 : 0); // guess not running strictly more than two servers
    if(!isFirstToRun)
    {
        while(isTCPPortInUse(serverPort))
            serverPort++;
        qInfo("serverPort: %hu", serverPort);
    }
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString peerAddress = settings.value("peerAddress").toString();

    Map map;
    
    if(runClient)
    {
        QString newPeerAddress = getText(QObject::tr("Peer address"), QObject::tr("A peer address"), isDefaultServerPortInUse ? QString::number(DEFAULT_SERVER_PORT) : peerAddress);
        if(isAPositiveInteger(newPeerAddress))
            newPeerAddress = "127.0.0.1:" + newPeerAddress; // localhost binds to ::1
        qInfo() << "peerAddress:" << newPeerAddress;
        if(peerAddress != newPeerAddress)
        {
            settings.setValue("peerAddress", QVariant(newPeerAddress));
            settings.sync();
        }
        peerAddress = newPeerAddress;
    }
	else
	{
        // If not client ask for the map to play on
		QStringList mapsStr = getAllCleanMapsStr();
        map = getAllMaps()[getQUInt8(QObject::tr("Map choice"), QObject::tr("What map do you want to play on ?"), mapsStr)];

        qInfo() << "map:" << getMapName(map);
    }

    // could give a shot to UPnP
    bool useInternetOpenPort = getBool(QObject::tr("Autoconfiguration"), runClient ? QObject::tr("Is your port %1 opened to others or are you the last to join the game ?").arg(QString::number(serverPort)) : QObject::tr("Is your port %1 opened to others ?").arg(QString::number(serverPort)));
    QProcess* myProcess;
    qint64 processId;
    if(!useInternetOpenPort)
    {
        // used https://askubuntu.com/a/50000/1560657 and https://askubuntu.com/a/583153/1560657 for server configuration
        // this aim is that anybody can use a VPS open port to the internet in order to host his server part in the P2P network
        // because not root no access to ports < 1024 and other ports require password for interaction
        // any shell access is disabled
        // could add restrictions on server side like restrict for a given IP 5 ports etc
        // hope that by using a random port nobody will use be using it
        // what if someone try to take a port already used by AUD ?
        // the password is just used to avoid massive SSH bots to try to do some bad behavior automatically (like using all ports)
        quint16 remotePort = 10000 + QRandomGenerator::global()->bounded(50000);
        qInfo() << "remotePort:" << remotePort;

        QStringList arguments;
        arguments << "-N" << "-R" << QString::number(remotePort) + ":localhost:" + QString::number(serverPort) << "anonymous@lemnoslife.com";

        QProcess* myProcess = new QProcess(inGameUI);
        myProcess->start("ssh", arguments);
        processId = myProcess->processId();
        qInfo() << "ssh program" << processId;
    }

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
        qInfo() << "Server started on:" << serverPort;
    }
    if(runClient)
    {
        discoverClient(peerAddress);
        qInfo("Waiting for discovery...");
        sleepWithEvents(TIME_S_ASSUME_DISCOVERED);
        qInfo("Discovered %d peers !", getPeers().size());
    }

    QStringList nicknames;
    // likewise if using multiple client for developing for instance we don't have problems
    QString nickname,
            oldNickname = isDefaultServerPortInUse ? "" : settings.value("nickname").toString();
    while(true)
    {
        nickname = getText(QObject::tr("Nickname"), QObject::tr("Your nickname"), oldNickname);
        qInfo() << "nickname:" << nickname;

        if(!runClient)
            break;

        qInfo("Asking for map...");
        QString mapStr = askAll("map"); // or should more precisely ask all nicknames at each nickname test ? but this assume to wait the maximum ping of someone ?
        qInfo() << "Received map: " << mapStr;

        map = getMap(mapStr);

        qInfo("Waiting for nicknames...");
        QString nicknamesStr = askAll("nicknames"); // or should more precisely ask all nicknames at each nickname test ? but this assume to wait the maximum ping of someone ?
        qInfo("Received nicknames: %s", nicknamesStr.toStdString().c_str());
        nicknames = nicknamesStr.split(",");
        QStringList realNicknames;
        for(QString nicknameStr : nicknames)
        {
            QString nickname = nicknameStr.section(' ', 1);
            realNicknames.push_back(nickname);
        }
        if(realNicknames.contains(nickname))
            showWarningMessage(QObject::tr("Nickname"), QObject::tr("This nickname is already used!"));
        else
            break;
    }
    if(!isDefaultServerPortInUse && nickname != oldNickname)
    {
        settings.setValue("nickname", QVariant(nickname));
        settings.sync();
    }
    if(runClient)
        for(QString nicknameStr : nicknames)
        {
            QString peerAddress = nicknameStr.section(' ', 0, 0);
            QString nickname = nicknameStr.section(' ', 1);
            inGameUI->spawnOtherPlayer(peerAddress, nickname);
        }

    if(runClient)
        sendToAll("nickname " + nickname);

    inGameUI->initialize(nickname, map);
    inGameUI->resize(640, 480);
    inGameUI->showMaximized();

    int res = app.exec();

    if(!useInternetOpenPort)
    {
        qInfo() << "kill ssh program";
        #ifdef _WIN32
            const auto explorer = OpenProcess(PROCESS_TERMINATE, false, processId);
            TerminateProcess(explorer, 1);
            CloseHandle(explorer);
		#elif __linux__
			kill(processId, SIGTERM);
        #endif
    }

    return res;
}

QString toString(InGameGUI inGameGUI)
{
    switch(inGameGUI)
    {
        case IN_GAME_GUI_NONE:
            return "IN_GAME_GUI_NONE";
        case IN_GAME_GUI_FIX_WIRING:
            return "IN_GAME_GUI_FIX_WIRING";
        case IN_GAME_GUI_ASTEROIDS:
            return "IN_GAME_GUI_ASTEROIDS";
        case IN_GAME_GUI_ENTER_ID_CODE:
            return "IN_GAME_GUI_ENTER_ID_CODE";
        case IN_GAME_GUI_ALIGN_ENGINE:
            return "IN_GAME_GUI_ALIGN_ENGINE";
        case IN_GAME_GUI_MAP:
            return "IN_GAME_GUI_MAP";
        case IN_GAME_GUI_CAMERA:
            return "IN_GAME_GUI_CAMERA";
        case IN_GAME_GUI_VITALS:
            return "IN_GAME_GUI_VITALS";
        case IN_GAME_GUI_MEETING:
            return "IN_GAME_GUI_MEETING";
        case IN_GAME_GUI_WIN_CREWMATES:
            return "IN_GAME_GUI_WIN_CREWMATES";
        case IN_GAME_GUI_WIN_IMPOSTORS:
            return "IN_GAME_GUI_WIN_IMPOSTORS";
        case IN_GAME_GUI_VENT:
            return "IN_GAME_GUI_VENT";
    }
    return "";
}
