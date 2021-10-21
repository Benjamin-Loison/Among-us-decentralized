#include "Client.h"
#include "Server.h"

Client::Client()
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    //connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    //connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    //connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    messageSize = 0;

    socket->abort(); // On désactive les connexions précédentes s'il y en a
    // Orange is so bad port opening doesn't work anymore but DMZ does :'(
    socket->connectToHost("2a01:cb00:774:4300:a4ba:9926:7e3a:b6c1"/*"192.168.1.45"*//*"localhost"*//*"90.127.197.24"*//*"2a01:cb00:774:4300:531:8a76:deda:2b53"*//*a secret domain name*/, SERVER_PORT); // On se connecte au serveur demandé
}

// Envoi d'un message au serveur
void Client::sendToServer(QString messageToSend)
{
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    // On prépare le paquet à envoyer

    out << (quint16)0;
    out << messageToSend;
    out.device()->seek(0);
    out << (quint16)(paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet
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

    qInfo(("client received: " + receivedMessage).toStdString().c_str());

    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    messageSize = 0;
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
            qWarning(("Error: " + socket->errorString()).toStdString().c_str());
    }
}
