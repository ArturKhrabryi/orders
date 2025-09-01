#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <stdexcept>
#include <vector>
#include "CodeEan.hpp"
#include "DatabaseProductsModel.hpp"
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

struct SqlFetchProductError: SqlError
{
    SqlFetchProductError (const QSqlError& sqlError) :
        SqlError("Cannot fetch products", sqlError)
    {}
};

struct SqlAddProductError : SqlError
{
    SqlAddProductError(const QSqlError& sqlError) :
        SqlError("Cannot add product", sqlError)
    {}
};

struct SqlMoveToTrashError : SqlError
{
    SqlMoveToTrashError(const QSqlError& sqlError) :
        SqlError("Cannot move products to trash", sqlError)
    {}
};

struct ColumnIdx { int name, codeEan, quantity, unitCode; };

class Database
{
public:
    Database();
    ~Database(); 

    DatabaseProductsModel* getModel() noexcept { return &this->model; }

    std::optional<Product> fetchByCodeEan(const CodeEan& codeEan) const;
    std::vector<Product> fetchByName(const QString& name) const;
    std::vector<Product> fetch() const;

    void add(const Product& product);

    void moveToTrash(int id);
    void moveAllToTrash();

private:
    QSqlDatabase db;
    DatabaseProductsModel model;

    ColumnIdx getNameIndexes(const QSqlQuery& sqlQuery) const noexcept;
    Product fromSqlQuery(const QSqlQuery& sqlQuery, const ColumnIdx& indexes = { 1, 2, 3, 4 }) const noexcept;

    void createUnits();
    void createProducts();
    void createTrash();

    void transaction();
    void commit();
    ScopeExit makeRollbackGuard();
};

