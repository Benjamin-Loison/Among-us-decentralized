#include "Server.h"
#include <QTcpServer>
#include "main.h"
//#include <unistd.h> // only linux...

// should use IPv6 by default

/*QTcpServer* server;
QList<QTcpSocket*> clients;
quint16 messageSize;
void newConnection(),
     dataReceived(),
     clientDisconnected();*/
bool askingAll = false;
QMap<QTcpSocket*, quint16> peersPorts;

Server::Server(quint16 serverPort)
{
    server = new QTcpServer();
    if(!server->listen(QHostAddress::Any, serverPort))
    {
        qWarning("Server couldn't start for reason: %s", server->errorString().toStdString().c_str());
    }
    else
    {
        //qWarning("Server started !");
        //QObject::connect(server, &QTcpServer::newConnection, [](){ newConnection(); });
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }

    messageSize = 0;
}

void Server::newConnection()
{
    QTcpSocket* newClient = server->nextPendingConnection();
    clients << newClient;

    //QObject::connect(newClient, &QIODevice::readyRead, [](){ dataReceived(); });
    //QObject::connect(newClient, &QAbstractSocket::disconnected, [](){ clientDisconnected(); });
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
    qInfo("server received: %s", message.toStdString().c_str());
    message = processMessageServer(socket, message);

    //message = processMessage(message);
    sendToSocket(socket, message);

    // 2 : remise de la taille du message à 0 pour permettre la réception des futurs messages
    messageSize = 0;
}

// assume not same message a second time until others validated
QMap<QPair<QString, QString>, QMap<QString, QString>> waitingMessages; // QMap<QPair<peerAddress, message>, verificatorsAddresses>

void processMessageCommon(QTcpSocket* socket, QString messagePart)
{
    QString socketString = socketToString(socket);
    if(messagePart.startsWith("position "))
    {
        messagePart = messagePart.replace("position ", "");
        QStringList coordinates = messagePart.split(' ');
        quint32 x = coordinates[0].toUInt(), y = coordinates[1].toUInt();
        inGameUI->movePlayer(socketString, x, y);
        //inGameUI->redraw(); // what if called a lot of time ?
    }
    else if(messagePart == "ready")
    {
        inGameUI->setPlayerReady(socketString);
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
        /*if(messagePart.startsWith("nickname "))
        {
            QString otherPlayeNickname = messagePart.replace("nickname ", "");
            inGameUI->spawnOtherPlayer(otherPlayeNickname);
            res += "nickname " + inGameUI->currPlayer.nickname;
        }*/
        if(messagePart.startsWith("discovering ")) // messagePart == "discovering"
        {
            QString remotePort = messagePart.replace("discovering ", "");
            peersPorts[socket] = remotePort.toUInt();
            //inGameUI->spawnOtherPlayer(otherPlayeNickname);
            QList<QTcpSocket*> peers = getPeers();
            quint16 peersSize = /*clients*/peers.size();//,
                    //clientsTreated = 0;
            if(peersSize > 1)
            {
                res += "peers ";
                QStringList fullAddresses;
                for(quint16 peersIndex = 0; peersIndex < peersSize; peersIndex++)
                {
                    QTcpSocket* currentSocket = peers[peersIndex]; // should also work with clients in InGameUI
                    if(currentSocket != socket)
                    {
                        quint16 currentPort = peersPorts[currentSocket];
                        QString peerFullAddress = socketToString(currentSocket);
                        QStringList parts = peerFullAddress.split(':');
                        parts.removeLast();
                        parts.append(QString::number(currentPort));
                        peerFullAddress = parts.join(':');
                        fullAddresses.push_back(peerFullAddress);
                        /*res += peerFullAddress;
                        clientsTreated++;
                        if(clientsTreated < clientsSize - 2) // warning unsigned etc
                        {
                            res += " ";
                        }*/
                    }
                }
                res += fullAddresses.join(' ');
            }
            //res += "nickname " + inGameUI->currPlayer.nickname; // no not trustable data until
        }
        else if(messagePart == "nicknames")
        {
            QStringList parts;
            parts.append(serverSocketToString() + " " + inGameUI->currPlayer.nickname);
            QList<QTcpSocket*> peers = getPeers();
            quint8 peersSize = peers.size();
            for(quint8 peersIndex = 0; peersIndex < peersSize; peersIndex++)
            {
                QTcpSocket* peer = peers[peersIndex];
                if(peer == socket) continue;
            //}
            //QStringList addresses = inGameUI->otherPlayers.keys();
            //quint8 addressesSize = addresses.size();
            //for(quint8 addressesIndex = 0; addressesIndex < addressesSize; addressesIndex++)
            //{
                QString address = socketToString(peer),//addresses[addressesIndex],
                        nickname = inGameUI->otherPlayers[address].nickname;
                QStringList addressParts = address.split(':');
                addressParts.removeLast();
                addressParts.append(QString::number(peersPorts[peer]));
                address = addressParts.join(':'); // could make a function for this purpose
                parts.append(address + " " + nickname);
            }
            /// order matters ! should do alphabetical one for instance
            res += parts.join(','); // should also send others nicknames, order is important here
        }
        else if(messagePart.startsWith("nickname "))
        {
            // What difference QList/QVector ?
            QString otherPlayerNickname = messagePart.replace("nickname ", "");
            inGameUI->spawnOtherPlayer(socketToString(socket), otherPlayerNickname); // could almost save players one per client/server and loop through this and not the list stored in InGameUI
        }
        else
        {
            processMessageCommon(socket, messagePart);
        }
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
    qInfo("sending to %s: %s!", socketString.toStdString().c_str(), messageToSend.toStdString().c_str());
    if(messageToSend == "") return; //messageToSend = EMPTY_NETWORK_RESPONSE; // warning user injection...
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    // On prépare le paquet à envoyer

    out << (quint16)0;
    out << messageToSend;
    out.device()->seek(0);
    out << (quint16)(paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet
}

QMap<QString, QString> askingAllMessages;
quint16 askingAllMessagesCounter = 0;

QString askAll(QString message)
{
    QList<QTcpSocket*> peers = getPeers();
    //QStringList verificatorsAddresses;
    quint16 peersSize = peers.size();
    askingAllMessagesCounter = peersSize;
    for(quint16 peersIndex = 0; peersIndex < peersSize; peersIndex++)
    {
        QTcpSocket* peer = peers[peersIndex];
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
        //usleep(1000);
        QCoreApplication::processEvents();
        qInfo("msleep 1");
        QThread::msleep(1);
        //if(waitingMessages[key].empty())
        //    break;
    }
    //return "";
    quint16 askingAllMessagesSize = askingAllMessages.size();
    QMap<QString, quint16> scores;
    QList<QString> askingAllMessagesValues = askingAllMessages.values();
    for(quint16 askingAllMessagesIndex = 0; askingAllMessagesIndex < askingAllMessagesSize; askingAllMessagesIndex++)
    {
        QString askingMessage = askingAllMessagesValues[askingAllMessagesIndex];
        if(scores.find(askingMessage) != scores.end())
            scores[askingMessage]++;
        else
            scores[askingMessage] = 1;
    }
    askingAllMessages.clear();
    // askingAllMessagesCounter already null
    QString majorMessage;
    quint16 scoresSize = scores.size(), maxScore = 0;
    QList<QString> scoresKeys = scores.keys();
    for(quint16 scoresIndex = 0; scoresIndex < scoresSize; scoresIndex++)
    {
        QString currentScoreKey = scoresKeys[scoresIndex];
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
    quint16 clientsSize = clients.size();
    for(quint16 clientsIndex = 0; clientsIndex < clientsSize; clientsIndex++)
    {
        Client* client = clients[clientsIndex];
        QTcpSocket* clientSocket = client->socket;
        res.append(clientSocket);
    }
    return res;
}

QString addressPortToString(QHostAddress address, quint16 port)
{
    QString addressStr = address.toString().replace("0.0.0.0", "127.0.0.1");
    if(shareIP != "")
            addressStr = addressStr.replace("127.0.0.1", shareIP);
    return addressStr.replace("::ffff:", "")/*.replace("::ffff:127.0.0.1", "127.0.0.1")*//*sometimes there is the prefix ::ffff:127.0.0.1*//*not sure about this*/ + ":" + QString::number(port);
}

QString serverSocketToString()
{
    return addressPortToString(server->server->serverAddress(), server->server->serverPort());
}

QString socketToString(QTcpSocket* socket)
{
    return addressPortToString(socket->peerAddress(), socket->peerPort());
}

void sendToAll(QString message)
{
    QList<QTcpSocket*> peers = getPeers();
    // can't do this in a single line ?
    quint16 peersSize = peers.size();
    for(quint16 peersIndex = 0; peersIndex < peersSize; peersIndex++)
    {
        QTcpSocket* peer = peers[peersIndex];
        sendToSocket(peer, message);
    }
}
