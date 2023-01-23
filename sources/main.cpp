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
bool useInternetOpenPort;
QTranslator translator;
quint16 remotePort = DEFAULT_SERVER_PORT;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QImageReader::setAllocationLimit(512);
    #endif
	
    #if __APPLE__
        QDir::setCurrent("../../../");
    #endif

    QString locale = QLocale::system().name().section('_', 0, 0),
            languageFile = "AmongUsDecentralizedFR";
    if(!translator.load(QString("qt_") + locale, QLibraryInfo::
                        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                            path
                        #else
                            location
                        #endif
                        (QLibraryInfo::TranslationsPath))) // this is only going to work for Qt installed machines, no ?
        qInfo("qt locale language file couldn't be loaded !");
    if(!translator.load(languageFile))
        qInfo("languageFile couldn't be loaded !");
    app.installTranslator(&translator);

    inGameUI = new InGameUI();
    // Uncomment this to see to what type of objects various events are sent
    //app.installEventFilter(new DebugEventFilter);

    bool isDefaultServerPortInUse = isTCPPortInUse(DEFAULT_SERVER_PORT);
    if(!isDefaultServerPortInUse) // assume no two servers running at the same time even for development purpose
        isFirstToRun = getBool(QObject::tr("First to run"), QObject::tr("Are you the first to run for this game ?"));
    bool runServer = true, // not required for last player joining the party - could always open it, it doesn't cost a lot and if someone want to use this node it is possible
         runClient = !isFirstToRun; // why using another variable just for code flexibility ?
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
    QString peerAddress;
    if(runClient)
    {
        peerAddress = settings.value("peerAddress").toString();
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
        inGameUI->map = getAllMaps()[getQUInt8(QObject::tr("Map choice"), QObject::tr("What map do you want to play on ?"), mapsStr)];

        qInfo() << "map:" << getMapName(inGameUI->map);
    }

    // could give a shot to UPnP
	// btw if everybody is using autoconfiguration then using a remote machine with open ports to the internet is useless since could just redirect packets in a localhost manner on this remote machine
    useInternetOpenPort = (runClient && peerAddress.startsWith(DOMAIN_NAME)) || !getBool(QObject::tr("Autoconfiguration"), QObject::tr("Is your port %1 opened to others ?").arg(QString::number(serverPort)));
    qint64 processId;
    if(useInternetOpenPort)
    {
        // used https://askubuntu.com/a/50000 and https://askubuntu.com/a/583153 for server configuration
        // this aim is that anybody can use a VPS open port to the internet in order to host his server part in the P2P network
        // because not root no access to ports < 1024 and other ports require password for interaction
        // any shell access is disabled
        // could add restrictions on server side like restrict for a given IP 5 ports etc
        // hope that by using a random port nobody will use be using it
        // what if someone try to take a port already used by AUD ?
        // the password is just used to avoid massive SSH bots to try to do some bad behavior automatically (like using all ports)
        // could force showAddress to true if isFirstToRun but that would be not very appreciated for accustomed people who don't need that
        remotePort = 10000 + QRandomGenerator::global()->bounded(50000);
        qInfo() << "remotePort:" << remotePort;

        QString remotePortStr = QString::number(remotePort);
        QStringList arguments;
        arguments << "-o" << "StrictHostKeyChecking=no" << "-N" << "-R" << remotePortStr + ":localhost:" + QString::number(serverPort) << "anonymous@" DOMAIN_NAME;

        QProcess* myProcess = new QProcess(inGameUI);
        myProcess->start("ssh", arguments);
        processId = myProcess->processId();
        qInfo() << "ssh program" << processId;
        //showMessage(QObject::tr("IP and port to share to peers"), "Share to peers connecting to you: " DOMAIN_NAME ':' + remotePortStr);
        // could add an icon to copy-paste
    }

	// the new ones connect to the old ones
    // let's say that we try to discover everyone (in the specific case of this game)
    // I preferred the "wait for a certain time" approach to the "wait for a certain number" approach because someone could make it look like some of the participants (that he generated) represent the others
	// let's say that we execute a message containing information only when the majority approves it (in the precise case of this game, otherwise result of impossibility of FLP)
	// it is assumed that people allow time to elapse between network actions when necessary
	// or in relation to the previous line, let's make a model where each participant sends to the others just his actions (thus full confidence in the others)
	// practical example: game where players participate with referees checking that no one cheats
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
        QString mapStr = askAll("map"); // should askAll map and nicknames at the same time
        qInfo() << "Received map: " << mapStr;

        inGameUI->map = getMap(mapStr);

        qInfo("Waiting for nicknames...");
        QString nicknamesStr = askAll("nicknames"); // or should more precisely ask all nicknames at each nickname test ? but this assume to wait the maximum ping of someone ?
        qInfo("Received nicknames: %s", nicknamesStr.toStdString().c_str());
        nicknames = nicknamesStr.split(',');
        QStringList realNicknames;
        for(QString nicknameStr : nicknames)
        {
            QString nickname = nicknameStr.section(' ', 1);
            realNicknames.push_back(nickname);
        }
        if(realNicknames.contains(nickname))
            showWarningMessage(QObject::tr("Nickname"), QObject::tr("This nickname is already used !"));
        else
            break;
    }
    if(!isDefaultServerPortInUse && nickname != oldNickname)
    {
        settings.setValue("nickname", QVariant(nickname));
        settings.sync();
    }

    if(runClient)
    {
        for(QString nicknameStr : nicknames)
        {
            QString peerAddress = nicknameStr.section(' ', 0, 0);
            QString nickname = nicknameStr.section(' ', 1);
            inGameUI->spawnOtherPlayer(peerAddress, nickname);
        }
        sendToAll("nickname " + nickname);
    }

    inGameUI->initialize(nickname);
    inGameUI->resize(640, 480);
    inGameUI->showMaximized();

    int res = app.exec();

    if(useInternetOpenPort)
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
