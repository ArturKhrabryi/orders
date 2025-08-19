#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <stdexcept>
#include <vector>
#include "ScopeExit.hpp"


struct Product;

struct SqlError : std::runtime_error
{
    const QSqlError sqlError;

private:
    static std::string makeMessage(const QString& prefix, const QSqlError& sqlError)
    {
        QString message = QString("%1: %2 (%3 / %4)")
            .arg(prefix)
            .arg(sqlError.text())
            .arg(sqlError.driverText())
            .arg(sqlError.databaseText());

        return message.toStdString();
    } 

public:
    SqlError(const QString& prefix, const QSqlError& sqlError) : 
        std::runtime_error(SqlError::makeMessage(prefix, sqlError)),
        sqlError(sqlError)
    {}
};

struct SqlOpenDatabaseError : SqlError
{
    SqlOpenDatabaseError(const QSqlError& sqlError) :
        SqlError("Cannot open database", sqlError)
    {}
};

struct SqlEnableForeignKeysError : SqlError
{
    SqlEnableForeignKeysError(const QSqlError& sqlError) : 
        SqlError("Cannot enable foreign keys", sqlError)
    {}
};

struct SqlBeginTransactionError : SqlError
{
    SqlBeginTransactionError(const QSqlError& sqlError) :
        SqlError("Cannot start sql transaction", sqlError)
    {}
};

struct SqlCommitTransactionError : SqlError
{
    SqlCommitTransactionError(const QSqlError& sqlError) :
        SqlError("Cannot commit sql transaction", sqlError)
    {}
};

struct SqlCreationTableError : SqlError
{
    SqlCreationTableError(const QSqlError& sqlError) :
        SqlError("Cannot create table", sqlError)
    {}
};

struct SqlInsertionIntoTableError : SqlError
{
    SqlInsertionIntoTableError(const QSqlError& sqlError) :
        SqlError("Cannot insert into table", sqlError)
    {}
};

struct SqlCreationIndexError : SqlError
{
    SqlCreationIndexError(const QSqlError& sqlError) :
        SqlError("Cannot create index", sqlError)
    {}
};

struct SqlFetchProductsError : SqlError
{
    SqlFetchProductsError(const QSqlError& sqlError) :
        SqlError("Cannot fetch products", sqlError)
    {}
};

struct SqlAddProductError : SqlError
{
    SqlAddProductError(const QSqlError& sqlError) :
        SqlError("Cannot add product", sqlError)
    {}
};

class Database
{
public:
    Database();
    ~Database(); 

    std::vector<Product> fetchProducts() const;
    void addProduct(const Product& product);
    void moveProductToTrash(const Product& product);
    void moveAllProductsToTrash();

private:
    QSqlDatabase db;

    void createUnits();
    void createProducts();
    void createTrash();

    void transaction();
    void commit();
    ScopeExit makeRollbackGuard();
};

