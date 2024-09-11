#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpWorker = new UDPworker(this);
    udpWorker->InitSocket();

    connect(udpWorker, &UDPworker::sig_sendTimeToGUI, this, &MainWindow::DisplayTime);
    connect(udpWorker, &UDPworker::sig_sendDataToGUI, this, &MainWindow::DatagramInfo);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{

        QDateTime dateTime = QDateTime::currentDateTime();

        QByteArray dataToSend;
        QDataStream outStr(&dataToSend, QIODevice::WriteOnly);

        outStr << dateTime;

        udpWorker->SendDatagram(dataToSend);
        timer->start(TIMER_DELAY);

    });

    ui->pb_stop->setEnabled(false);


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pb_start_clicked()
{
    timer->start(TIMER_DELAY);

    ui->pb_stop->setEnabled(true);
    ui->pb_start->setEnabled(false);
}


void MainWindow::DisplayTime(QDateTime data)
{
    counterPck++;
    if(counterPck % 20 == 0){
        ui->te_result->clear();
    }

    ui->te_result->append("Текущее время: " + data.toString() + ". "
                "Принято пакетов " + QString::number(counterPck));


}

void MainWindow::on_pb_stop_clicked()
{
    timer->stop();

    ui->pb_start->setEnabled(true);
    ui->pb_stop->setEnabled(false);
}

void MainWindow::DatagramInfo(QString adress, long long size)
{
    ui->te_result->append("Принято сообщение от: " + adress + ", размером сообщения(байт) " + QString::number(size));
}


void MainWindow::on_pb_send_clicked()
{
    if(!ui->te_input->toPlainText().isEmpty()){
        QString strData = ui->te_input->toPlainText();
        QByteArray dataToSend;
        QDataStream outStr(&dataToSend, QIODevice::WriteOnly);
        outStr << strData;
        udpWorker->SendMyDatagram(dataToSend);
        ui->te_input->clear();
    }
}
