#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include "OrderTableModel.hpp"
#include "ScopeExit.hpp"


struct ProductFormData;
struct Product;
struct CodeEan;

struct Unit { QString unitCode, unitDescription; };

class Database
{
public:
    Database();
    ~Database(); 

    OrderTableModel* getModel() noexcept { return &this->model; }

    bool exists(const CodeEan& codeEan) const;

    void addOrderLine(const ProductFormData& productFormData);
    void updateColumn(const OrderTableModel::Row& row, OrderTableModel::ColumnName columnName, const QVariant& data);

    void moveOrderLinesToTrash();
    void moveOrderLineToTrash(int id);

    QSqlDatabase getDb() noexcept { return this->db; }

private:
    QSqlDatabase db;
    OrderTableModel model;

    static QSqlDatabase createDb();

    //return inserted product's id
    QVariant insertOrGetProduct(const Product& product);

    static void createUnits(QSqlDatabase& db);
    static void createProducts(QSqlDatabase& db);
    static void createOrderLines(QSqlDatabase& db);
    static void createTrash(QSqlDatabase& db);

    void transaction();
    void commit();
    ScopeExit makeRollbackGuard();
};

