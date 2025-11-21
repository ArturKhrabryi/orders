#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <vector>
#include "CodeEan.hpp"
#include "DatabaseProductsModel.hpp"
#include "ScopeExit.hpp"


struct Product;


struct ColumnIdx { int name, codeEan, quantity, unitCode; };
struct Unit { QString unitCode, unitDescription; };

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

