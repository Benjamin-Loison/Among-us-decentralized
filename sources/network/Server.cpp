#include "Server.h"
#include <QTcpServer>
#include "../main.h"
using namespace std;

// should use IPv6 by default

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
    // 1: we receive a packet (or a sub-packet) from one of the clients

    // We determine which client sends the message (search for the client's QTcpSocket)
    qInfo("dataReceived begin");
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket == 0) // If by chance we haven't found the client at the origin of the signal, we stop the method
        return;

    // If all goes well, we continue: we retrieve the message
    QDataStream in(socket);

    //qInfo("dataReceived"); // not executed even if message sent...
    if(messageSize == 0) // If we don't know the size of the message yet, we try to get it
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16)) // We did not receive the full size of the message
             return;

        in >> messageSize; // If we have received the entire size of the message, we retrieve it
    }

    // If we know the size of the message, we check if we have received the whole message
    if (socket->bytesAvailable() < messageSize) // If we haven't received everything yet, we stop the method
        return;

    // If these lines are executed, it means that we have received the whole message: we can recover it
    QString message;
    in >> message;
    qInfo() << "server received from" << socketToString(socket) << ":" << message; // not logging Position and keep logged all network stuff
    message = processMessageServer(socket, message);

    //message = processMessage(message);
    sendToSocket(socket, message);

    // 2: reset the message size to 0 to allow the reception of future messages
    messageSize = 0;
    if(socket->bytesAvailable() > 0)
    {
        qInfo("Server::dataReceived recursive was needed");
        dataReceived();
    }
    qInfo("dataReceived end");
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
    else if(messagePart.startsWith("Vent "))
    {
        player->isInvisible = messagePart.replace("Vent ", "") == "enter";
    }
    else if(messagePart.startsWith("RandomHashed "))
    {
        if(player->privateRandomHashed == "")
        {
            messagePart = messagePart.replace("RandomHashed ", "");
            player->privateRandomHashed = messagePart;
            inGameUI->waitingAnswersNumber--;
            /*if(needEverybodyReadyCall)
                inGameUI->onEverybodyReadySub(false);*/
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
        if(nickname == inGameUI->currPlayer.nickname)
            inGameUI->closeTask();
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
        inGameUI->setPlayerReady(socketString/*, true*/); // second argument is useless because this function is only called here
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
    else if(messagePart.startsWith("Sabotage_doors "))
    {
        const int prefixSize = QString("Sabotage_doors ").size();
        bool ok = false;
        QString roomIdString = messagePart.mid(prefixSize);
        uint iRoom = roomIdString.toUInt(&ok);
        if(!ok) {
            qWarning() << "Received invalid door sabotage message:" << roomIdString << "is not an unsigned integer";
            return;
        }
        if(uint(iRoom) >= uint(inGameUI->rooms.size())) {
            qWarning() << "Received invalid door sabotage message:" << iRoom << "is not a valid room ID, there are" << inGameUI->rooms.size() << "rooms";
            return;
        }
        inGameUI->rooms[iRoom].sabotage();
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
            QList<QTcpSocket*> peers = getPeers();
            quint16 peersSize = peers.size();
            if(peersSize > 1)
            {
                QStringList fullAddresses;
                for(QTcpSocket* currentSocket : peers)
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
                res += "peers " + fullAddresses.join(' ');
            }
            res = "YourAddress " + socketWithoutPortToString(socket) + (res != "" ? NETWORK_SEPARATOR + res : "");
            // doesn't solve the double NETWORK_SEPARATOR: sending to 192.168.1.20:45282: YourAddress 192.168.1.20#SEP##SEP#peers 192.168.1.55:10821!
        }
        else if(messagePart == "nicknames")
        {
            QMap<quint8, QString> sortedParts;
            Player* currPlayer = &inGameUI->currPlayer;
            sortedParts[currPlayer->playerId] = serverSocketToString() + " " + currPlayer->nickname;
            QList<QTcpSocket*> peers = getPeers();
            for(QTcpSocket* peer : peers)
            {
                if(peer == socket)
                    continue;

                QString address = socketToString(peer);
                Player* player = &inGameUI->otherPlayers[address];
                QString nickname = player->nickname;
                QStringList addressParts = address.split(':');
                addressParts.removeLast();
                addressParts.append(QString::number(peersPorts[peer]));
                address = addressParts.join(':'); // could make a function for this purpose - is the same thing used elsewhere ?
                sortedParts[player->playerId] = address + " " + nickname;
            }

            /// order matters especially a logical for the game one
            // QMap already sorted values by key
            res += messagePart + "|" + sortedParts.values().join(",");
        }
        else if(messagePart == "map")
        {
            QString mapStr = QString::number(inGameUI->map);
			qInfo() << "currPlayer" << mapStr;
            res += messagePart + "|" + mapStr;
        }
        else if(messagePart.startsWith("nickname "))
        {
            // What difference QList/QVector ?
            QString otherPlayerNickname = messagePart.replace("nickname ", "");
            inGameUI->spawnOtherPlayer(socketToString(socket), otherPlayerNickname); // could almost save players one per client/server and loop through this and not the list stored in InGameUI
            QPair<quint16, quint16> xy = SPAWNS[inGameUI->map];
            if(inGameUI->currPlayer.x != xy.first || inGameUI->currPlayer.y != xy.second)
            {
                res = inGameUI->currPlayer.getSendPositionMessage();
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
    // We determine which client disconnects
    qInfo("clientDisconnected");
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket == 0) // If by chance we haven't found the client at the origin of the signal, we stop the method
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

    // We prepare the packet to be sent

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

    if(socket->write(paquet) == -1) // We send the packet
        qInfo("socket write error");
    if(!socket->waitForBytesWritten())
        qInfo("wait for bytes error");
    socket->flush(); // maybe it's the solution https://doc.qt.io/qt-5/qabstractsocket.html#flush
    qInfo("sendToSocket end");
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
    return useInternetOpenPort ? DOMAIN_NAME : address.toString().replace("::ffff:", "").replace("0.0.0.0", myAddress/*"127.0.0.1"*/);//.replace("127.0.0.1", useInternetOpenPort ? DOMAIN_NAME : "127.0.0.1");
}

QString addressPortToString(QHostAddress address, quint16 port)
{
    QString addressStr = addressToString(address);
    return addressToString(address)/*.replace("::ffff:127.0.0.1", "127.0.0.1")*//*sometimes there is the prefix ::ffff:127.0.0.1*//*not sure about this*/ + ":" + QString::number(port);
}

QString serverSocketToString()
{
    return addressPortToString(server->server->serverAddress(), useInternetOpenPort ? remotePort : server->server->serverPort());
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
