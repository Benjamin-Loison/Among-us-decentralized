#include "Client.h"
#include "Server.h"
#include "../main.h"
using namespace std;

Client::Client(QString peerAddress) : socket(new QTcpSocket(this)), isConnected(false), messageSize(0)
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    socket->abort(); // Disable previous connections if there are any
    QStringList peerAddressParts = peerAddress.split(':');
    peerAddress = peerAddressParts[0];
    quint16 serverPort = peerAddressParts[1].toUInt();
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

// We received a packet (or a sub-packet)
void Client::dataReceived()
{
    /* Same principle as when the server receives a packet:
    We try to retrieve the size of the message
    Once we have it, we wait until we receive the whole message (based on the announced messageSize)*/

    qInfo("Client::dataReceived begin"); // no bug at this level
    QDataStream in(socket);

    if(messageSize == 0)
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16))
             return;
        in >> messageSize;
    }

    if(socket->bytesAvailable() < messageSize)
        return;

    // If we get to this line, we can recover the whole message
    QString receivedMessage;
    in >> receivedMessage;

    // got problem on client side this time (server sent but no client received...)
    qInfo() << "client received from" << socketToString(socket) << ':' << receivedMessage;
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

    // We reset the message size to 0 to be able to receive future messages
    messageSize = 0;
    //qInfo() << "bytesAvailable:" << socket->bytesAvailable();
    if(socket->bytesAvailable() > 0)
    {
        qInfo("Client::dataReceived recursive was needed");
        dataReceived();
    }
    qInfo("dataReceived end");
}

void Client::processMessageClient(QString message)
{
    QStringList messageParts = message.split(NETWORK_SEPARATOR);
    for(QString messagePart : messageParts)
    {
        if(askingAll && messagePart.contains('|')) // copied from higher level
        {
            QString peerString = socketToString(socket);
            if(askingAllMessages.contains(peerString) && askingAllMessages[peerString] == "")
            {
                askingAllMessages[peerString] = messagePart.section('|', 1, 1);
                askingAllMessagesCounter--;
                qInfo() << "askingAllMessagesCounter:" << askingAllMessagesCounter;
            }
            continue;
        }

        if(messagePart.startsWith("peers "))
        {
            QString connected = messagePart.replace("peers ", "");
            QStringList connectedParts = connected.split(' ');
            for(QString connectedPart : connectedParts)
                if(all_of(clients.begin(), clients.end(), [&](const Client* client) { return socketToString(client->socket) != connectedPart; }))
                    discoverClient(connectedPart);
        }
        else
            processMessageCommon(socket, messagePart);
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
    if(!peerAddress.contains(':'))
        peerAddress += ':' + QString::number(DEFAULT_SERVER_PORT);
    Client* client = new Client(peerAddress);
    clients.push_back(client);
    QStringList peerAddressParts = peerAddress.split(':');
    peersPorts[client->socket] = peerAddressParts.last().toUInt();
    client->sendToServer("YourAddress " + socketWithoutPortToString(client->socket) + NETWORK_SEPARATOR + "discovering " + QString::number(remotePort)/*serverSocketToString().split(':').last()*/);
}
