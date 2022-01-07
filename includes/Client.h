#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H

#include <QtWidgets>
#include <QtNetwork>

class Client : public QWidget
{
    Q_OBJECT

    public:
        Client(QString peerAddress);
        void sendToServer(QString messageToSend);
        QTcpSocket* socket;
        bool isConnected;
        void processMessageClient(QString message);

    private slots:
        void dataReceived();
        void connecte();
        void deconnecte();
        void socketError(QAbstractSocket::SocketError error);

    private:
        quint16 messageSize;
};

void discoverClient(QString peerAddress);

#endif
