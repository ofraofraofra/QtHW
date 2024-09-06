#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Исходное состояние виджетов
    ui->setupUi(this);
    ui->lb_statusConnect->setStyleSheet("color:red");
    ui->pb_request->setEnabled(false);

    /*
     * Выделим память под необходимые объекты. Все они наследники
     * QObject, поэтому воспользуемся иерархией.
    */

    dataDb = new DbData(this);
    dataBase = new DataBase(this);
    msg = new QMessageBox(this);
    empty_model = new QSqlTableModel(this);
    //Установим размер вектора данных для подключения к БД
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);

    /*
     * Добавим БД используя стандартный драйвер PSQL и зададим имя.
    */
    dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);

    /*
     * Устанавливаем данные для подключениея к БД.
     * Поскольку метод небольшой используем лямбда-функцию.
     */
    connect(dataDb, &DbData::sig_sendData, this, [&](QVector<QString> receivData){
        dataForConnect = receivData;
    });

    /*
     * Соединяем сигнал, который передает ответ от БД с методом, который отображает ответ в ПИ
     */
    connect(dataBase, qOverload<QSqlTableModel*, int>(&DataBase::sig_SendDataFromDB), this, qOverload<QSqlTableModel*, int>(&MainWindow::ScreenDataFromDB));
    connect(dataBase, qOverload<QSqlQueryModel*, int>(&DataBase::sig_SendDataFromDB), this, qOverload<QSqlQueryModel*, int>(&MainWindow::ScreenDataFromDB));

    /*
     *  Сигнал для подключения к БД
     */
    connect(dataBase, &DataBase::sig_SendStatusConnection, this, &MainWindow::ReceiveStatusConnectionToDB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * @brief Слот отображает форму для ввода данных подключения к БД
 */
void MainWindow::on_act_addData_triggered()
{
    //Отобразим диалоговое окно. Какой метод нужно использовать?
    dataDb->show();
}

/*!
 * @brief Слот выполняет подключение к БД. И отображает ошибки.
 */

void MainWindow::on_act_connect_triggered()
{
    /*
     * Обработчик кнопки у нас должен подключаться и отключаться от БД.
     * Можно привязаться к надписи лейбла статуса. Если он равен
     * "Отключено" мы осуществляем подключение, если "Подключено" то
     * отключаемся
    */

    if(ui->lb_statusConnect->text() == "Отключено"){

       ui->lb_statusConnect->setText("Подключение");
       ui->lb_statusConnect->setStyleSheet("color : black");

       auto conn = [&]{dataBase->ConnectToDataBase(dataForConnect);};
       QtConcurrent::run(conn);

    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        ui->lb_statusConnect->setText("Отключено");
        ui->act_connect->setText("Подключиться");
        ui->lb_statusConnect->setStyleSheet("color:red");
        ui->pb_request->setEnabled(false);
    }
}

/*!
 * \brief Обработчик кнопки "Получить"
 */
void MainWindow::on_pb_request_clicked()
{

    switch (ui->cb_category->currentIndex()) {

        case requestHorrors:{
            request = "SELECT title, description FROM film f JOIN film_category fc on f.film_id = fc.film_id JOIN category c on c.category_id = fc.category_id WHERE c.name = 'Horror'";
            break;
        }
        case requestComedy:{
            request = "SELECT title, description FROM film f JOIN film_category fc on f.film_id = fc.film_id JOIN category c on c.category_id = fc.category_id WHERE c.name = 'Comedy'";
            break;
        }
        default:
                break;
    }
    auto req = [&]{dataBase->RequestToDB(request, ui->tableView, ui->cb_category->currentIndex());};
    QtConcurrent::run(req);
    ///Тут должен быть код ДЗ

}

/*!
 * \brief Слот отображает значение в QTableWidget
 * \param widget
 * \param typeRequest
 */
void MainWindow::ScreenDataFromDB(QSqlQueryModel *model, int typeRequest)
{
    ui->tableView->setModel(model);
    ui->tableView->showColumn(0);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::ScreenDataFromDB(QSqlTableModel *model, int typeRequest)
{
    ui->tableView->setModel(model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->hideColumn(0);
    for(int i = 3; i < ui->tableView->model()->columnCount(); ++i)
    {
        ui->tableView->hideColumn(i);
    }
}
/*!
 * \brief Метод изменяет стотояние формы в зависимости от статуса подключения к БД
 * \param status
 */
void MainWindow::ReceiveStatusConnectionToDB(bool status)
{
    if(status){
        ui->act_connect->setText("Отключиться");
        ui->lb_statusConnect->setText("Подключено к БД");
        ui->lb_statusConnect->setStyleSheet("color:green");
        ui->pb_request->setEnabled(true);
    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        msg->setIcon(QMessageBox::Critical);
        msg->setText(dataBase->GetLastError().text());
        ui->lb_statusConnect->setText("Отключено");
        ui->lb_statusConnect->setStyleSheet("color:red");
        msg->exec();
    }
}

void MainWindow::on_pb_clear_clicked()
{
    ui->tableView->setModel(empty_model);
}
