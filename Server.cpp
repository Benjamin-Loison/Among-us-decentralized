#include "Server.h"
#include <QTcpServer>
#include "main.h"
#include <WorkerThread.h>
using namespace std;

// should use IPv6 by default
// could say what ip and port received data from for client and server

bool askingAll = false, needEverybodyReadyCall = false;
QMap<QTcpSocket*, quint16> peersPorts;
// assume not same message a second time until others validated
QMap<QPair<QString, QString>, QMap<QString, QString>> waitingMessages; // QMap<QPair<peerAddress, message>, verificatorsAddresses>
QMap<QString, QString> askingAllMessages;
quint16 askingAllMessagesCounter = 0;

Server::Server(quint16 serverPort) : server(new QTcpServer()), messageSize(0)
{
    if(!server->listen(QHostAddress::Any, serverPort))
        qWarning("Server couldn't start for reason: %s", server->errorString().toStdString().c_str());
    else
    {
        //qWarning("Server started !");
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
}

void Server::newConnection()
{
    QTcpSocket* newClient = server->nextPendingConnection();
    clients << newClient;

    connect(newClient, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
}

void Server::dataReceived()
{
    // 1 : on reçoit un paquet (ou un sous-paquet) d'un des clients

    // On détermine quel client envoie le message (recherche du QTcpSocket du client)
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;

    // Si tout va bien, on continue : on récupère le message
    QDataStream in(socket);

    //qInfo("dataReceived"); // not executed even if message sent...
    if(messageSize == 0) // Si on ne connaît pas encore la taille du message, on essaie de la récupérer
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16)) // On n'a pas reçu la taille du message en entier
             return;

        in >> messageSize; // Si on a reçu la taille du message en entier, on la récupère
    }

    // Si on connaît la taille du message, on vérifie si on a reçu le message en entier
    if (socket->bytesAvailable() < messageSize) // Si on n'a pas encore tout reçu, on arrête la méthode
        return;

    // Si ces lignes s'exécutent, c'est qu'on a reçu tout le message : on peut le récupérer !
    QString message;
    in >> message;
    qInfo("server received: %s", message.toStdString().c_str()); // not logging Position and keep logged all network stuff
    message = processMessageServer(socket, message);

    //message = processMessage(message);
    sendToSocket(socket, message);

    // 2 : remise de la taille du message à 0 pour permettre la réception des futurs messages
    messageSize = 0;
    if(socket->bytesAvailable() > 0)
    {
        qInfo("Server::dataReceived recursive was needed");
        dataReceived();
    }
}

void processMessageCommon(QTcpSocket* socket, QString messagePart)
{
    QString socketString = socketToString(socket);
    Player* player = inGameUI->otherPlayers.contains(socketString) ? &inGameUI->otherPlayers[socketString] : nullptr;
    if(messagePart.startsWith("Position ")) // using uppercase allow us to make quickly sure that it is the start of a sentence
    {
        messagePart = messagePart.replace("Position ", "");
        QStringList coordinates = messagePart.split(' ');
        quint32 x = coordinates[0].toUInt(), y = coordinates[1].toUInt();
        inGameUI->movePlayer(socketString, x, y);
    }
    else if(messagePart == "Facing left")
    {
        inGameUI->setFacingLeftPlayer(socketString);
    }
    else if(messagePart == "Skip" || messagePart.startsWith("Voted "))
    {
        inGameUI->executeVote(messagePart, player);
    }
    else if(messagePart == "Proceed_vote")
    {
        if(inGameUI->meetingResultsWidget)
            inGameUI->meetingResultsWidget->onProceed();
    }
    else if(messagePart.startsWith("RandomHashed "))
    {
        if(player->privateRandomHashed == "")
        {
            messagePart = messagePart.replace("RandomHashed ", "");
            player->privateRandomHashed = messagePart;
            inGameUI->waitingAnswersNumber--;
            /*if(needEverybodyReadyCall)
            {
                inGameUI->onEverybodyReadySub(false);
            }*/
        }
    }
    else if(messagePart.startsWith("finished "))
    {
        messagePart = messagePart.replace("finished ", "");
        TaskTime taskTime = getTaskTime(messagePart);
        inGameUI->taskFinished(taskTime);
        inGameUI->checkEndOfTheGame();
    }
    else if(messagePart.startsWith("Kill "))
    {
        const int prefixSize = QString("Kill ").size();
        QString nickname = messagePart.mid(prefixSize);
        Player* player = inGameUI->getPlayer(nickname);
        inGameUI->killPlayer(*player);
        inGameUI->checkEndOfTheGame();
    }
    else if(messagePart.startsWith("Imposter "))
    {
        messagePart = messagePart.replace("Imposter ", "");
        inGameUI->setImpostor(messagePart);
    }
    else if(messagePart == "ready")
    {
        inGameUI->setPlayerReady(socketString, true); // second argument is useless because this function is only called here
        //WorkerThread* workerThread = new WorkerThread();
        //workerThread->start();
    }
    else if(messagePart.startsWith("Report "))
    {
        const int prefixSize = QString("Report ").size();
        QString nickname = messagePart.mid(prefixSize);
        Player* reportedPlayer = inGameUI->getPlayer(nickname);
        inGameUI->openMeetingUI(reportedPlayer, player);
    }
    else if(messagePart == "Emergency meeting")
    {
        inGameUI->openMeetingUI(nullptr, player);
    }
    else if(messagePart.startsWith("YourAddress "))
    {
        messagePart = messagePart.replace("YourAddress ", "");
        myAddress = messagePart; // should take the majority here too
    }
}

QString Server::processMessageServer(QTcpSocket* socket, QString message)
{
    QStringList messageParts = message.split(NETWORK_SEPARATOR);
    quint32 messagePartsSize = messageParts.size();
    QString res = "";
    for(quint32 messagePartsIndex = 0; messagePartsIndex < messagePartsSize; messagePartsIndex++)
    {
        QString messagePart = messageParts[messagePartsIndex];
        if(messagePart.startsWith("discovering "))
        {
            QString remotePort = messagePart.replace("discovering ", "");
            peersPorts[socket] = remotePort.toUInt();
            QList<QTcpSocket*> sockets = peersPorts.keys();
            for(QTcpSocket* s : sockets)
            {
                qInfo() << "socket" << socketToString(s) << peersPorts[s];
            }
            QList<QTcpSocket*> peers = getPeers();
            quint16 peersSize = peers.size();
            if(peersSize > 1)
            {
                QStringList fullAddresses;
                for(QTcpSocket* currentSocket : peers)
                {
                    if(currentSocket != socket)
                    {
                        quint16 currentPort = peersPorts[currentSocket];
                        QString peerFullAddress = socketToString(currentSocket);
                        QStringList parts = peerFullAddress.split(':');
                        parts.removeLast();
                        parts.append(QString::number(currentPort));
                        peerFullAddress = parts.join(':');
                        fullAddresses.push_back(peerFullAddress);
                    }
                }
                res += "peers " + fullAddresses.join(' ');
            }
            res = "YourAddress " + socketWithoutPortToString(socket) + (res != "" ? NETWORK_SEPARATOR + res : "");
            // doesn't solve the double NETWORK_SEPARATOR: sending to 192.168.1.20:45282: YourAddress 192.168.1.20#SEP##SEP#peers 192.168.1.55:10821!
        }
        else if(messagePart == "nicknames")
        {
            QStringList parts;
            parts.append(serverSocketToString() + " " + inGameUI->currPlayer.nickname);
            QList<QTcpSocket*> peers = getPeers();
            for(QTcpSocket* peer : peers)
            {
                if(peer == socket) continue;

                QString address = socketToString(peer),
                        nickname = inGameUI->otherPlayers[address].nickname;
                QStringList addressParts = address.split(':');
                addressParts.removeLast();
                addressParts.append(QString::number(peersPorts[peer]));
                address = addressParts.join(':'); // could make a function for this purpose - is the same thing used elsewhere ?
                parts.append(address + " " + nickname);
            }
            /// order matters ! should do alphabetical one for instance
            res += messagePart + "|" + parts.join(',');
        }
        else if(messagePart.startsWith("nickname "))
        {
            // What difference QList/QVector ?
            QString otherPlayerNickname = messagePart.replace("nickname ", "");
            inGameUI->spawnOtherPlayer(socketToString(socket), otherPlayerNickname); // could almost save players one per client/server and loop through this and not the list stored in InGameUI
            if(inGameUI->currPlayer.x != X_SPAWN || inGameUI->currPlayer.y != Y_SPAWN)
            {
                res = "Position " + QString::number(inGameUI->currPlayer.x) + " " + QString::number(inGameUI->currPlayer.y);
                if(inGameUI->currPlayer.playerFacingLeft)
                    res += NETWORK_SEPARATOR"Facing left";
            }
        }
        else
            processMessageCommon(socket, messagePart);
        if(messagePartsIndex < messagePartsSize - 1)
            res += NETWORK_SEPARATOR;
    }
    return res;
}

void Server::clientDisconnected()
{
    // On détermine quel client se déconnecte
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;

    clients.removeOne(socket);

    socket->deleteLater();
}

void sendToSocket(QTcpSocket* socket, QString messageToSend)
{
    QString socketString = socketToString(socket);
    if(messageToSend == "") return; //messageToSend = EMPTY_NETWORK_RESPONSE; // warning user injection...
    qInfo("sending to %s: %s!", socketString.toStdString().c_str(), messageToSend.toStdString().c_str());
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    // On prépare le paquet à envoyer

    out << (quint16)0;
    out << messageToSend;
    QIODevice* device = out.device();
    if(device == nullptr)
        qInfo("device nullptr");
    if(!device->seek(0))
        qInfo("seek error");
    int size = paquet.size();
    out << (quint16)(size - sizeof(quint16));
    //qInfo() << "size:" << size << "device:" << device;

    if(socket->write(paquet) == -1) // On envoie le paquet
        qInfo("socket write error");
    if(!socket->waitForBytesWritten())
        qInfo("wait for bytes error");
    socket->flush(); // maybe it's the solution https://doc.qt.io/qt-5/qabstractsocket.html#flush
    // still having the problem (without error message)
    // the syncing problem seem to really be at sending step because dataReceived isn't ever triggered when there is the bug
    // flushing and waitForReadyRead may be interesting ? https://forum.qt.io/topic/46323/solved-qtcpsocket-would-not-receiving-all-data/2
    // it doesn't seem to be exactly the last packet but last few
    // I don't know if during dataReceived we also receive the forgotten message or something and we should recursively loop ?
}

QString askAll(QString message)
{
    QList<QTcpSocket*> peers = getPeers();
    //QStringList verificatorsAddresses;
    askingAllMessagesCounter = peers.size();
    for(QTcpSocket* peer : peers)
    {
        QString peerString = socketToString(peer);
        //verificatorsAddresses.push_back(peerString);
        sendToSocket(peer, message);
        askingAllMessages[peerString] = ""; // assume always returns a non empty string
    }
    askingAll = true;
    //QPair<QString, QString> key = qMakePair("localhost", "askAll");
    ///waitingMessages[key] = verificatorsAddresses;
    //return "";
    while(askingAllMessagesCounter > 0)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
        //if(waitingMessages[key].empty())
        //    break;
    }
    //return "";
    QMap<QString, quint16> scores;
    QList<QString> askingAllMessagesValues = askingAllMessages.values();
    for(QString askingMessage : askingAllMessagesValues)
        scores[askingMessage]++; // no problem even if key not set
    askingAllMessages.clear();
    // askingAllMessagesCounter already null
    QString majorMessage;
    quint16 maxScore = 0;
    QList<QString> scoresKeys = scores.keys();
    for(QString currentScoreKey : scoresKeys)
    {
        quint16 currentScore = scores[currentScoreKey];
        if(currentScore > maxScore/* || maxScore == 0*/) // not necessary
        {
            maxScore = currentScore;
            majorMessage = currentScoreKey;
        }
    }
    return majorMessage;
}

QList<QTcpSocket*> getPeers()
{
    QList<QTcpSocket*> res;
    if(server != nullptr) // if code was clear shouldn't be here
        res.append(server->clients);
    for(Client* client : clients)
        res.append(client->socket);
    return res;
}

QString addressToString(QHostAddress address)
{
    return address.toString().replace("::ffff:", "").replace("0.0.0.0", myAddress/*"127.0.0.1"*/);
}

QString addressPortToString(QHostAddress address, quint16 port)
{
    QString addressStr = addressToString(address);
    return addressToString(address)/*.replace("::ffff:127.0.0.1", "127.0.0.1")*//*sometimes there is the prefix ::ffff:127.0.0.1*//*not sure about this*/ + ":" + QString::number(port);
}

QString serverSocketToString()
{
    return addressPortToString(server->server->serverAddress(), server->server->serverPort());
}

QString socketWithoutPortToString(QTcpSocket* socket)
{
    return addressToString(socket->peerAddress());
}

QString socketToString(QTcpSocket* socket)
{
    return addressPortToString(socket->peerAddress(), socket->peerPort());
}

typedef struct sendToAllStruct {
    QTcpSocket* peer;
    void operator() (QString messagePart) {sendToSocket(peer, messagePart);}
} sendToAllStruct;

void sendToAll(QString message)
{
    QList<QTcpSocket*> peers = getPeers();

    for(QTcpSocket* peer : peers)
    {
        //sendToSocket(peer, message); // doesn't used to have the following even if it isn't optimized, it works
        QStringList messageParts = message.split(NETWORK_SEPARATOR);
        sendToAllStruct sendToAllStructInstance;
        sendToAllStructInstance.peer = peer;
        for_each(messageParts.begin(), messageParts.end(), sendToAllStructInstance);
    }
}
