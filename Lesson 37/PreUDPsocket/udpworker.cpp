#include "udpworker.h"

UDPworker::UDPworker(QObject *parent) : QObject(parent){}

void UDPworker::InitSocket()
{

    serviceUdpSocket = new QUdpSocket(this);
    newUdpSocket = new QUdpSocket(this);

    serviceUdpSocket->bind(QHostAddress::LocalHost, BIND_PORT);
    newUdpSocket->bind(QHostAddress::LocalHost, BIND_PORT_2);

    connect(serviceUdpSocket, &QUdpSocket::readyRead, this, &UDPworker::readPendingDatagrams);
    connect(newUdpSocket, &QUdpSocket::readyRead, this, &UDPworker::readMyDatagrams);
}

void UDPworker::ReadDatagram(QNetworkDatagram datagram)
{

    QByteArray data;
    data = datagram.data();
    QDataStream inStr(&data, QIODevice::ReadOnly);
    QDateTime dateTime;
    inStr >> dateTime;

    emit sig_sendTimeToGUI(dateTime);
}

void UDPworker::ReadMyDatagram(QNetworkDatagram datagram)
{
    QByteArray data;
    data = datagram.data();
    QString adress = datagram.senderAddress().toString();
    qint64 size = data.size();

    emit sig_sendDataToGUI(adress,size);
}

void UDPworker::SendDatagram(QByteArray data)
{
    serviceUdpSocket->writeDatagram(data, QHostAddress::LocalHost, BIND_PORT);
}

void UDPworker::SendMyDatagram(QByteArray data)
{
    newUdpSocket->writeDatagram(data, QHostAddress::LocalHost, BIND_PORT_2);
}

void UDPworker::readPendingDatagrams( void )
{
    while(serviceUdpSocket->hasPendingDatagrams()){
            QNetworkDatagram datagram = serviceUdpSocket->receiveDatagram();
            ReadDatagram(datagram);
    }

}

void UDPworker::readMyDatagrams(void)
{
    while(newUdpSocket->hasPendingDatagrams()){
        QNetworkDatagram datagram = newUdpSocket->receiveDatagram();
        ReadMyDatagram(datagram);
    }
}
