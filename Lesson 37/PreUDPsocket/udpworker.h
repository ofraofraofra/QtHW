#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDateTime>


#define BIND_PORT 12345
#define BIND_PORT_2 23456

class UDPworker : public QObject
{
    Q_OBJECT
public:
    explicit UDPworker(QObject *parent = nullptr);
    void InitSocket( void );
    void ReadDatagram( QNetworkDatagram datagram);
    void SendDatagram(QByteArray data );

    void ReadMyDatagram( QNetworkDatagram datagram);
    void SendMyDatagram(QByteArray data);

private slots:
    void readPendingDatagrams(void);

    void readMyDatagrams(void);

private:
    QUdpSocket* serviceUdpSocket;

    QUdpSocket* newUdpSocket;

signals:
    void sig_sendTimeToGUI(QDateTime data);

    void sig_sendDataToGUI(QString adress, qint64 size);

};

#endif // UDPWORKER_H
