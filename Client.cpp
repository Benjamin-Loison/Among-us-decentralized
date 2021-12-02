#include "Client.h"
#include "Server.h"
#include "main.h"

Client::Client(QString peerAddress)
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    messageSize = 0;
    isConnected = false;

    socket->abort(); // On désactive les connexions précédentes s'il y en a
    quint16 serverPort = DEFAULT_SERVER_PORT;
    if(peerAddress.contains(':'))
    {
        QStringList peerAddressParts = peerAddress.split(':');
        //quint8 peerAddressPartsSize = peerAddressParts.size();
        quint16 potentialPort = peerAddressParts.last().toUInt();
        if(potentialPort > 255)
        {
            serverPort = potentialPort;
        }
        peerAddressParts.removeLast();
        peerAddress = peerAddressParts.join(':');
    }
    qInfo() << "client connecting to" << peerAddress << "on port" << serverPort << "...";
    socket->connectToHost(peerAddress, serverPort);
    // could wait connected before logging discovering otherwise IP is incorrect
    // however it assumes we will connect to him quickly which may not be the case
    while(!isConnected) // looping seems important instead of single call to processEvents, doing this bloquant way assume that all remote peers trying to be connected to will succeed quickly or will have to wait for timeout (if there is any)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }
}

void Client::connecte()
{
    isConnected = true;
    qInfo("connecte");
}

void Client::deconnecte()
{
    qInfo("deconnecte");
}

void Client::sendToServer(QString messageToSend) // not very useful in theory...
{
    sendToSocket(socket, messageToSend);
}

// On a reçu un paquet (ou un sous-paquet)
void Client::dataReceived()
{
    /* Même principe que lorsque le serveur reçoit un paquet :
    On essaie de récupérer la taille du message
    Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée messageSize)*/
    QDataStream in(socket);

    if(messageSize == 0)
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16))
             return;
        in >> messageSize;
    }

    if(socket->bytesAvailable() < messageSize)
        return;

    // Si on arrive jusqu'à cette ligne, on peut récupérer le message entier
    QString receivedMessage;
    in >> receivedMessage;

    qInfo() << "client received:" << receivedMessage;
    /*if(askingAll)
    {
        QString peerString = socketToString(socket);
        if(askingAllMessages.find(peerString) != askingAllMessages.end() && askingAllMessages[peerString] == "") // second condition in order not to someone to revoke what he claimed
        {
            askingAllMessages[peerString] = receivedMessage;
            askingAllMessagesCounter--;
            qInfo() << "askingAllMessagesCounter:" << askingAllMessagesCounter;
        }
        else
            processMessageClient(receivedMessage);
    }
    else*/
        processMessageClient(receivedMessage);

    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    messageSize = 0;
}

void Client::processMessageClient(QString message)
{
    QStringList messageParts = message.split(NETWORK_SEPARATOR);
    quint32 messagePartsSize = messageParts.size();
    for(quint32 messagePartsIndex = 0; messagePartsIndex < messagePartsSize; messagePartsIndex++)
    {
        QString messagePart = messageParts[messagePartsIndex];
        if(askingAll) // copied from higher level
        {
            if(messagePart.contains('|'))
            {
                QString peerString = socketToString(socket);
                if(askingAllMessages.find(peerString) != askingAllMessages.end() && askingAllMessages[peerString] == "")
                {
                    askingAllMessages[peerString] = messagePart.split('|')[1];
                    askingAllMessagesCounter--;
                    qInfo() << "askingAllMessagesCounter:" << askingAllMessagesCounter;
                }
                continue;
            }
        }


        if(messagePart.startsWith("peers "))
        {
            QString connected = messagePart.replace("peers ", "");
            QStringList connectedParts = connected.split(" ");
            quint32 connectedPartsSize = connectedParts.size();
            for(quint32 connectedPartsIndex = 0; connectedPartsIndex < connectedPartsSize; connectedPartsIndex++)
            {
                QString connectedPart = connectedParts[connectedPartsIndex];
                quint8 clientsSize = clients.size();
                bool alreadyInContacts = false;
                for(quint8 clientsIndex = 0; clientsIndex < clientsSize; clientsIndex++)
                {
                    Client* client = clients[clientsIndex];
                    QString clientAddress = socketToString(client->socket);
                    if(clientAddress == connectedPart)
                    {
                        alreadyInContacts = true;
                        break;
                    }
                }
                if(!alreadyInContacts)
                    discoverClient(connectedPart);
            }
        }
        /*else if(messagePart.startsWith("nickname "))
        {
            QString otherPlayeNickname = messagePart.replace("nickname ", "");
            inGameUI->spawnOtherPlayer(otherPlayeNickname);
        }*/
        else
        {
            processMessageCommon(socket, messagePart);
        }
    }
}

void Client::socketError(QAbstractSocket::SocketError error) // not used
{
    switch(error)
    {
        case QAbstractSocket::HostNotFoundError:
            qWarning("Error: the server couldn't be found. Check IP and port.");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qWarning("Error: the server refused the connection.");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            qWarning("Error: the server shutdown the connection.");
            break;
        default:
            qWarning() << "Error:" << socket->errorString();
    }
}

void discoverClient(QString peerAddress)
{
    Client* client = new Client(peerAddress);
    clients.push_back(client);
    QStringList peerAddressParts = peerAddress.split(':');
    peersPorts[client->socket] = peerAddressParts.last().toUInt();
    //client->sendToServer("nickname " + nickname);
    //client->sendToServer("discovering " + inGameUI->currPlayer.nickname);
    //peerAddressParts.removeLast();
    client->sendToServer("YourAddress " + /*peerAddressParts.join(':')*/socketWithoutPortToString(client->socket) + NETWORK_SEPARATOR + "discovering " + serverSocketToString().split(':').last() /*+ inGameUI->currPlayer.nickname*/);
}
