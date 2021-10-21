#include "Server.h"
#include <QTcpServer>

// should use IPv6 by default

/*QTcpServer* server;
QList<QTcpSocket*> clients;
quint16 messageSize;

void newConnection(),
     dataReceived(),
     clientDisconnected();*/

Server::Server()
{
    server = new QTcpServer();
    if(!server->listen(QHostAddress::Any, SERVER_PORT))
    {
        qWarning(("Server couldn't start for reason: " + server->errorString()).toStdString().c_str());
    }
    else
    {
        qWarning("Server started !");
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
    ///connect(newClient, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
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
    qInfo(("server received: " + message).toStdString().c_str());

    message = "hello client !";
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16)0; // On écrit 0 au début du paquet pour réserver la place pour écrire la taille
    out << message; // On ajoute le message à la suite
    out.device()->seek(0); // On se replace au début du paquet
    out << (quint16)(paquet.size() - sizeof(quint16)); // On écrase le 0 qu'on avait réservé par la longueur du message

    socket->write(paquet);

    // 2 : remise de la taille du message à 0 pour permettre la réception des futurs messages
    messageSize = 0;
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
