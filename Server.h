#ifndef DEF_SERVER_H
#define DEF_SERVER_H

#include <QtWidgets>
#include <QtNetwork>

#define DEFAULT_SERVER_PORT 10821
#define NETWORK_SEPARATOR "#SEP#"
#define EMPTY_NETWORK_RESPONSE "#EMPTY#"
#define TIME_S_ASSUME_DISCOVERED 1

class Server : public QWidget
{
    Q_OBJECT

    public:
        Server(quint16 serverPort = DEFAULT_SERVER_PORT);
        QList<QTcpSocket*> clients;
        QTcpServer* server;

    private slots:
        void newConnection();
        void dataReceived();
        void clientDisconnected();

    private:
        quint16 messageSize;
        QString processMessageServer(QTcpSocket* socket, QString message);
};

extern bool askingAll;
extern QMap<QString, QString> askingAllMessages;
extern quint16 askingAllMessagesCounter;
void sendToSocket(QTcpSocket* socket, QString messageToSend), sendToAll(QString message);
QString askAll(QString message), /*addressPortToString(QHostAddress address, quint16 port)*/serverSocketToString(), socketToString(QTcpSocket* socket);
QList<QTcpSocket*> getPeers();

#endif
