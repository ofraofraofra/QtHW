#ifndef DATABASE_H
#define DATABASE_H

#include <QTableWidget>
#include <QObject>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQueryModel>

#define POSTGRE_DRIVER "QPSQL"
#define DB_NAME "MyDB"

//Количество полей данных необходимых для подключения к БД
#define NUM_DATA_FOR_CONNECT_TO_DB 5

//Перечисление полей данных
enum fieldsForConnect{
    hostName = 0,
    dbName = 1,
    login = 2,
    pass = 3,
    port = 4
};

//Типы запросов
enum requestType{

    requestAllFilms,
    requestComedy,
    requestHorrors

};

class DataBase : public QObject
{
    Q_OBJECT

public:
    explicit DataBase(QObject *parent = nullptr);
    ~DataBase();

    void AddDataBase(QString driver, QString nameDB = "");
    void DisconnectFromDataBase(QString nameDb = "");
    void RequestToDB(QString request, QTableView *view, int requestType);
    QSqlError GetLastError(void);
    void ConnectToDataBase(QVector<QString> dataForConnect);


signals:

   void sig_SendDataFromDB(QSqlQueryModel *query_model, int typeR);
   void sig_SendDataFromDB(QSqlTableModel *table_model, int typeR);
   void sig_SendStatusConnection(bool);



private:

    QSqlDatabase* dataBase;
    QSqlTableModel *table_model = nullptr;
    QSqlQueryModel *query_model;
};

#endif // DATABASE_H