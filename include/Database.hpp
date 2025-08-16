#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <vector>
#include "ScopeExit.hpp"


struct Product;

class Database
{
public:
    Database();
    ~Database(); 

    std::vector<Product> fetchProducts() const;
    void addProduct(const Product& product);

private:
    QSqlDatabase db;

    void transaction();
    void commit();
    ScopeExit makeRollbackGuard();
};

