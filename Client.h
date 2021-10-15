#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H

#include <QtWidgets>
#include <QtNetwork>

class Client : public QWidget
{
    Q_OBJECT

    public:
        Client();

    private slots:
        void sendToServer();
        void dataReceived();
        void socketError(QAbstractSocket::SocketError error);

    private:
        QTcpSocket* socket;
        quint16 messageSize;
};


#endif
