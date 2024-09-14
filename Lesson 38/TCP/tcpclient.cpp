#include "tcpclient.h"

QDataStream &operator >>(QDataStream &out, ServiceHeader &data){

    out >> data.id;
    out >> data.idData;
    out >> data.status;
    out >> data.len;
    return out;
};
QDataStream &operator <<(QDataStream &in, ServiceHeader &data){

    in << data.id;
    in << data.idData;
    in << data.status;
    in << data.len;

    return in;
};


TCPclient::TCPclient(QObject *parent) : QObject(parent)
{
   socket = new QTcpSocket(this);
   connect(socket, &QTcpSocket::readyRead, this, &TCPclient::ReadyReed);
   connect(socket, &QTcpSocket::connected, this, [&]{
        emit sig_connectStatus(STATUS_SUCCES);
    });
   connect(socket, &QTcpSocket::errorOccurred, this, [&]{
         emit sig_connectStatus(ERR_CONNECT_TO_HOST);
     });
   connect(socket, &QTcpSocket::disconnected, this, &TCPclient::sig_Disconnected);
}

void TCPclient::SendRequest(ServiceHeader head)
{
    QByteArray arr_send;
    QDataStream outStream(&arr_send, QIODevice::WriteOnly);
    head.len = 0;
    outStream << head;
    socket->write(arr_send);
}


void TCPclient::SendData(ServiceHeader head, QString str)
{
    QByteArray arr_send;
    QDataStream outStream(&arr_send, QIODevice::WriteOnly);

    head.len = str.size();
    if (head.len == 0)
        return;

    outStream << head;
    outStream << str;
    socket->write(arr_send);

}


void TCPclient::ConnectToHost(QHostAddress host, uint16_t port)
{
    socket->connectToHost(host, port);
}

void TCPclient::DisconnectFromHost()
{
    socket->disconnectFromHost();
}


void TCPclient::ReadyReed()
{
    QDataStream incStream(socket);
    if(incStream.status() != QDataStream::Ok){
        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Ошибка открытия входящего потока для чтения данных!");
        msg.exec();
    }


    //Читаем до конца потока
    while(incStream.atEnd() == false){
        //Если мы обработали предыдущий пакет, мы скинули значение idData в 0
        if(servHeader.idData == 0){

            //Проверяем количество полученных байт. Если доступных байт меньше чем
            //заголовок, то выходим из обработчика и ждем новую посылку. Каждая новая
            //посылка дописывает данные в конец буфера
            if(socket->bytesAvailable() < sizeof(ServiceHeader)){
                return;
            }
            else{
                //Читаем заголовок
                incStream >> servHeader;
                //Проверяем на корректность данных. Принимаем решение по заранее известному ID
                //пакета. Если он "битый" отбрасываем все данные в поисках нового ID.
                if(servHeader.id != ID){
                    uint16_t hdr = 0;
                    while(incStream.atEnd()){
                        incStream >> hdr;
                        if(hdr == ID){
                            incStream >> servHeader.idData;
                            incStream >> servHeader.status;
                            incStream >> servHeader.len;
                            break;
                        }
                    }
                }
            }
        }
        //Если получены не все данные, то выходим из обработчика. Ждем новую посылку
        if(socket->bytesAvailable() < servHeader.len){
            return;
        }
        else{
            //Обработка данных
            ProcessingData(servHeader, incStream);
            servHeader.idData = 0;
            servHeader.status = 0;
            servHeader.len = 0;
        }
    }
}


void TCPclient::ProcessingData(ServiceHeader header, QDataStream &stream) //
{

    switch (header.idData){

        case GET_TIME:
        {
            QDateTime time;
            stream >> time;
            emit sig_sendTime(time);
            break;
        }
        case GET_SIZE:
        {
            uint32_t free_size;
            stream >> free_size;
            emit sig_sendFreeSize(free_size);
            break;
        }
        case GET_STAT:
        {
            StatServer stat;
            stream >> stat.incBytes;
            stream >> stat.sendBytes;
            stream >> stat.revPck;
            stream >> stat.sendPck;
            stream >> stat.workTime;
            stream >> stat.clients;
            emit sig_sendStat(stat);
            break;
        }
        case SET_DATA:
        {
            QString data_str;
            stream >> data_str;
            emit  sig_SendReplyForSetData(data_str);
            break;
        }
        case CLEAR_DATA:
        {
            if (header.status == STATUS_SUCCES)
            {
                 emit  sig_Success(CLEAR_DATA);
            }
            break;
        }
        default:
            return;

        }
}
