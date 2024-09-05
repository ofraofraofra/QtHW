#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphic.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , graphicWindow(nullptr)
{
    ui->setupUi(this);

    calcTime = 0;
    calcTimer = new QTimer(this);

    connect(calcTimer, &QTimer::timeout, this, [this]{
        ui->statusbar->showMessage(QString("Расчет длится %1 секунд").arg(calcTime));
        calcTime += 0.5;
    });

    connect(this, &MainWindow::sig_resultAv, this, [this]{
        DisplayResult(mins, maxs);
    });

    connect(this, &MainWindow::sig_graphReady, this, &MainWindow::on_graphReady);

    connect(this, &MainWindow::sig_dataReady, this, [this]{

    emit sig_graphReady(processData.mid(0, FD));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graphicWindow;
}

/****************************************************/
/*!
@brief:	Метод считывает данные из файла
@param: path - путь к файлу
        numberChannel - какой канал АЦП считать
*/
/****************************************************/
QVector<uint32_t> MainWindow::ReadFile(QString path, uint8_t numberChannel)
{

    QFile file(path);
    file.open(QIODevice::ReadOnly);

    if(file.isOpen() == false){

        if(pathToFile.isEmpty()){
            QMessageBox mb;
            mb.setWindowTitle("Ошибка");
            mb.setText("Ошибка открытия фала");
            mb.exec();
        }
    }


    QDataStream dataStream;
    dataStream.setDevice(&file);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    QVector<uint32_t> readData;
    readData.clear();
    uint32_t currentWorld = 0, sizeFrame = 0;

    while(dataStream.atEnd() == false){

        dataStream >> currentWorld;

        if(currentWorld == 0xFFFFFFFF){

            dataStream >> currentWorld;

            if(currentWorld < 0x80000000){

                dataStream >> sizeFrame;

                if(sizeFrame > 1500){
                    continue;
                }

                for(int i = 0; i<sizeFrame/sizeof(uint32_t); i++){

                    dataStream >> currentWorld;

                    if((currentWorld >> 24) == numberChannel){

                        readData.append(currentWorld);

                    }
                }
            }
        }
    }
    ui->chB_readSucces->setChecked(true);
    return readData;
}

QVector<double> MainWindow::ProcessFile(const QVector<uint32_t> dataFile)
{
    QVector<double> resultData;
    resultData.clear();

    foreach (int word, dataFile) {
        word &= 0x00FFFFFF;
        if(word > 0x800000){
            word -= 0x1000000;
        }

        double res = ((double)word/6000000)*10;
        resultData.append(res);
    }
    ui->chB_procFileSucces->setChecked(true);
    return resultData;
}

QVector<double> MainWindow::FindMax(QVector<double> resultData)
{
    double max = 0, sMax=0;
    //Поиск первого максиума
    QThread::sleep(1);
    foreach (double num, resultData){
        //QThread::usleep(1);
        if(num > max){
            max = num;
        }
    }

    //Поиск 2го максимума
    QThread::sleep(1);
    foreach (double num, resultData){
        if(num > sMax && (qFuzzyCompare(num, max) == false)){
            sMax = num;
        }
    }

    QVector<double> maxs = {max, sMax};
    ui->chB_maxSucess->setChecked(true);
    return maxs;
}

QVector<double> MainWindow::FindMin(QVector<double> resultData)
{

    double min = 0, sMin = 0;
    QThread::sleep(1);
    //Поиск первого максиума
    foreach (double num, resultData){
        if(num < min){
            min = num;
        }
    }
    QThread::sleep(1);
    //Поиск 2го максимума
    foreach (double num, resultData){
        if(num < sMin && (qFuzzyCompare(num, min) == false)){
            sMin = num;
        }
    }

    QVector<double> mins = {min, sMin};
    ui->chB_minSucess->setChecked(true);
    return mins;

}

void MainWindow::DisplayResult(QVector<double> mins, QVector<double> maxs)
{
    calcTimer->stop( );
    ui->statusbar->showMessage(QString("Расчет закончен за %1 секунд").arg(calcTime));
    calcTime = 0;
    ui->te_Result->append("Расчет закончен!");

    ui->te_Result->append("Первый минимум " + QString::number(mins.first()));
    ui->te_Result->append("Второй минимум " + QString::number(mins.at(1)));

    ui->te_Result->append("Первый максимум " + QString::number(maxs.first()));
    ui->te_Result->append("Второй максимум " + QString::number(maxs.at(1)));
}


/****************************************************/
/*!
@brief:	Обработчик клика на кнопку "Выбрать путь"
*/
/****************************************************/
void MainWindow::on_pb_path_clicked()
{
    pathToFile = "";
    ui->le_path->clear();

    pathToFile =  QFileDialog::getOpenFileName(this,
                                              tr("Открыть файл"), "/home/", tr("ADC Files (*.adc)"));

    ui->le_path->setText(pathToFile);
}

/****************************************************/
/*!
@brief:	Обработчик клика на кнопку "Старт"
*/
/****************************************************/
void MainWindow::on_pb_start_clicked()
{
    //проверка на то, что файл выбран
    if(pathToFile.isEmpty()){

        QMessageBox mb;
        mb.setWindowTitle("Ошибка");
        mb.setText("Выберите файл!");
        mb.exec();
        return;
    }

    ui->chB_maxSucess->setChecked(false);
    ui->chB_procFileSucces->setChecked(false);
    ui->chB_readSucces->setChecked(false);
    ui->chB_minSucess->setChecked(false);

    int selectIndex = ui->cmB_numCh->currentIndex();
    //Маски каналов
    if(selectIndex == 0){
        numberSelectChannel = 0xEA;
    }
    else if(selectIndex == 1){
        numberSelectChannel = 0xEB;
    }
    else if(selectIndex == 2){
        numberSelectChannel = 0xEC;
    }
    calcTimer->start(500);
    ui->statusbar->showMessage("Расчет запущен");

    auto datRead = [this]{ return ReadFile(pathToFile, numberSelectChannel); };
    auto procData = [this](QVector<uint32_t> rData){ return ProcessFile(rData); };

    auto find = [this](QVector<double> pData) {
        QVector<double> firstSecondData = pData.mid(0, FD);
        maxs = FindMax(pData);
        mins = FindMin(pData);

        emit sig_graphReady(firstSecondData);
        emit sig_resultAv();
    };

    QtConcurrent::run(datRead).then(procData).then(find);
}



void MainWindow::on_graphReady(QVector<double> firstSecondData)
{
    if (!graphicWindow || !graphicWindow->isVisible()) {
            if (graphicWindow) {
                delete graphicWindow;
            }
            graphicWindow = new Graphic(this);
            graphicWindow->show();
        }

    QCustomPlot *customPlot = graphicWindow->findChild<QCustomPlot*>("customPlot");

    QVector<double> x(FD);
    for (int i = 0; i < FD; ++i)
        x[i] = i;

    if (customPlot->graphCount() == 0) {
        customPlot->addGraph();
    }

    customPlot->graph(0)->setData(x, firstSecondData);
    customPlot->rescaleAxes();
    customPlot->replot();

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->axisRect()->setRangeZoomFactor(0.9);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
}

void MainWindow::initializeGraphicWindow()
{
    if (!graphicWindow || !graphicWindow->isVisible()) {
        if (graphicWindow) {
            delete graphicWindow;
        }
        graphicWindow = new Graphic(this);
        graphicWindow->show();
    }
}
