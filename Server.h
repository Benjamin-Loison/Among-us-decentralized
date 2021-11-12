#ifndef DEF_SERVER_H
#define DEF_SERVER_H

#include <QtWidgets>
#include <QtNetwork>

#define DEFAULT_SERVER_PORT 10821
#define NETWORK_SEPARATOR "#SEP#"
#define TIME_S_ASSUME_DISCOVERED 3

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

void sendToSocket(QTcpSocket* socket, QString messageToSend);
QString askAll(QString message), socketToString(QTcpSocket* socket);
QList<QTcpSocket*> getPeers();

#endif
