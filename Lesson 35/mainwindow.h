#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDataStream>
#include <QMessageBox>
#include <QtGlobal>
#include <algorithm>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>

#include "qcustomplot.h"
#include "graphic.h"

#define FD 1000.0 //частота дискретизации

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        //Метод считывает данные из исходного файла
        QVector<uint32_t> ReadFile(QString path, uint8_t numberChannel);
        //Метод преобразует данные физические величины, В?
        QVector<double> ProcessFile(QVector<uint32_t> dataFile);
        //Метод ищет Максимумы
        QVector<double> FindMax(QVector<double> resultData);
        //Метод ищет минимумы
        QVector<double> FindMin(QVector<double> resultData);
        //Метод отображает результаты
        void DisplayResult(QVector<double> mins, QVector<double> maxs);

private slots:
    void on_pb_path_clicked();
    void on_pb_start_clicked();
    void on_graphReady(QVector<double> firstSecondData);
signals:
    void sig_resultAv();
    void sig_graphReady(QVector<double> firstSecondData);
    void sig_dataReady();

private:
    Ui::MainWindow *ui;
    QString pathToFile = "";
    uint8_t numberSelectChannel = 0xEA;
    QTimer* calcTimer;
    double calcTime;

    QVector<uint32_t> readData;
    QVector<double> processData;
    QVector<double> mins, maxs;

    Graphic *graphicWindow;

    void initializeGraphicWindow();
};
#endif // MAINWINDOW_H
