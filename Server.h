#ifndef DEF_SERVER_H
#define DEF_SERVER_H

#include <QtWidgets>
#include <QtNetwork>

#define SERVER_PORT 10821

class Server : public QWidget
{
    Q_OBJECT

    public:
        Server();

    private slots:
        void newConnection();
        void dataReceived();
        void clientDisconnected();

    private:
        QTcpServer* server;
        QList<QTcpSocket*> clients;
        quint16 messageSize;
};

#endif
