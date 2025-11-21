#include "Database.hpp"

#include <QString>
#include <QSqlRecord>
#include <QCoreApplication>
#include <stdexcept>
#include <vector>
#include "Product.hpp"
#include "DatabaseError.hpp"


Database::Database() :
    db(QSqlDatabase::addDatabase("QSQLITE")),
    model(nullptr, db)
{
    this->db.setDatabaseName("order.db");
    if (!this->db.open())
        throw SqlOpenDatabaseError(this->db.lastError());

    QSqlQuery cur(this->db);
    if (!cur.exec("PRAGMA foreign_keys = ON"))
        throw SqlEnableForeignKeysError(cur.lastError());

    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    this->createUnits();
    this->createProducts();
    this->createTrash();

    this->commit();
    rollbackGuard.release();
    
    this->model.setTable("products");
    this->model.setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!this->model.select())
        throw std::runtime_error(model.lastError().text().toStdString());

    this->model.setHeaderData(0, Qt::Horizontal, "Id");
    this->model.setHeaderData(1, Qt::Horizontal, DatabaseProductsModel::tr("Name"));
    this->model.setHeaderData(2, Qt::Horizontal, DatabaseProductsModel::tr("Code ean"));
    this->model.setHeaderData(3, Qt::Horizontal, DatabaseProductsModel::tr("Quantity"));
    this->model.setHeaderData(4, Qt::Horizontal, DatabaseProductsModel::tr("Unit"));
}

Database::~Database()
{
    if (this->db.isOpen())
        this->db.close();
}

std::optional<Product> Database::fetchByCodeEan(const CodeEan& codeEan) const
{
    QString sql = "SELECT name, codeEan, quantity, unitCode FROM products WHERE codeEan=?";
    QSqlQuery cur(this->db);
    cur.setForwardOnly(true);
    cur.prepare(sql);
    cur.addBindValue(codeEan.getValue());

    if (!cur.exec())
        throw SqlFetchProductError(cur.lastError());

    if (!cur.next())
        return std::nullopt;

    auto indexes = this->getNameIndexes(cur);
    
    return this->fromSqlQuery(cur, indexes);
}

std::vector<Product> Database::fetchByName(const QString& name) const
{
    QString sql = "SELECT name, codeEan, quantity, unitCode FROM products WHERE name LIKE ?";
    QSqlQuery cur(this->db);
    cur.setForwardOnly(true);
    cur.prepare(sql);
    cur.addBindValue("%" + name + "%");

    if (!cur.exec())
        throw SqlFetchProductError(cur.lastError());

    std::vector<Product> products;
    auto indexes = this->getNameIndexes(cur);
    while (cur.next())
        products.push_back(this->fromSqlQuery(cur, indexes));
    
    return products;
}

std::vector<Product> Database::fetch() const
{
    QSqlQuery cur(this->db); 
    cur.setForwardOnly(true);

    const QString sql = "SELECT name, codeEan, quantity, unitCode FROM products ORDER BY id ASC";
    if (!cur.exec(sql))
        throw SqlFetchProductError(cur.lastError());

    std::vector<Product> products;
    products.reserve(64);
    auto indexes = this->getNameIndexes(cur);
    while (cur.next())
    {
        auto product = this->fromSqlQuery(cur, indexes);

        products.push_back(std::move(product));
    }

    return products;
}

void Database::add(const Product& product)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();


    QString sql =
        "INSERT INTO products (name, codeEan, quantity, unitCode) "
        "VALUES (?, ?, ?, ?) "
        "ON CONFLICT(codeEan) "
        "DO UPDATE "
        "SET quantity = quantity + excluded.quantity";

    QSqlQuery cur(this->db);
    cur.prepare(sql);
    cur.addBindValue(product.name);
    cur.addBindValue(product.codeEan.has_value() ? product.codeEan->getValue() : QVariant()); 
    cur.addBindValue(product.quantity);
    cur.addBindValue(product.unitCode);

    if (!cur.exec())
        throw SqlAddProductError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::moveToTrash(int id)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    QString sql = "INSERT INTO trash (name, codeEan, quantity, unitCode) SELECT name, codeEan, quantity, unitCode FROM products WHERE id=?";
    QSqlQuery cur(this->db);
    cur.prepare(sql);
    cur.addBindValue(id);

    if (!cur.exec())
        throw SqlMoveToTrashError(cur.lastError());

    sql = "DELETE FROM products where id=?";
    cur.prepare(sql);
    cur.addBindValue(id);

    if (!cur.exec())
        throw SqlMoveToTrashError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::moveAllToTrash()
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    QString sql = "INSERT INTO trash (name, codeEan, quantity, unitCode) SELECT name, codeEan, quantity, unitCode FROM products";
    QSqlQuery cur(this->db);
    if (!cur.exec(sql))
        throw SqlMoveToTrashError(cur.lastError());

    sql = "DELETE FROM products";
    if (!cur.exec(sql))
        throw SqlMoveToTrashError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

ColumnIdx Database::getNameIndexes(const QSqlQuery& sqlQuery) const noexcept
{
    const auto rec = sqlQuery.record();
    auto iName = rec.indexOf("name");
    auto iCodeEan = rec.indexOf("codeEan");
    auto iQuantity = rec.indexOf("quantity");
    auto iUnitCode = rec.indexOf("unitCode");

    return { iName, iCodeEan, iQuantity, iUnitCode };
}

Product Database::fromSqlQuery(const QSqlQuery& sqlQuery, const ColumnIdx& indexes) const noexcept
{
    const auto& [iName, iCodeEan, iQuantity, iUnitCode] = indexes;

    Product product;
    product.name = sqlQuery.value(iName).toString();
    std::optional<CodeEan> codeEan = std::nullopt;
    auto codeEanValue = sqlQuery.value(iCodeEan);
    if (!codeEanValue.isNull() && !codeEanValue.toString().isEmpty())
        codeEan = CodeEan(sqlQuery.value(iCodeEan).toString());
    product.codeEan = std::move(codeEan);
    product.quantity = sqlQuery.value(iQuantity).toFloat();
    product.unitCode = sqlQuery.value(iUnitCode).toString();

    return product;
}

void Database::createUnits()
{
    QString sql =
        "CREATE TABLE IF NOT EXISTS units (\n"
            "\tcode TEXT PRIMARY KEY,\n"
            "\tname TEXT NOT NULL\n"
            ") WITHOUT ROWID";

    QSqlQuery cur(this->db);
    if (!cur.exec(sql))
        throw SqlCreationTableError(cur.lastError());

    const std::vector<Unit> units = {

        { QCoreApplication::translate("DatabaseUnits", "st"),
            QCoreApplication::translate("DatabaseUnits", "set") },

        { QCoreApplication::translate("DatabaseUnits", "pkg"),
            QCoreApplication::translate("DatabaseUnits", "packaging") },

        { QCoreApplication::translate("DatabaseUnits", "pal"),
            QCoreApplication::translate("DatabaseUnits", "palette") },

        { QCoreApplication::translate("DatabaseUnits", "pc"),
            QCoreApplication::translate("DatabaseUnits", "piece") },

        { QCoreApplication::translate("DatabaseUnits", "t"),
            QCoreApplication::translate("DatabaseUnits", "ton") },

        { QCoreApplication::translate("DatabaseUnits", "m2"),
            QCoreApplication::translate("DatabaseUnits", "square meter") },

        { QCoreApplication::translate("DatabaseUnits", "rl"),
            QCoreApplication::translate("DatabaseUnits", "roll") }
    };

    sql = QStringLiteral("INSERT OR IGNORE INTO units (code, name) VALUES\n");
    for (const auto& unit : units)
    {
        sql += QStringLiteral("\t('%1', '%2'),\n").arg(unit.unitCode).arg(unit.unitDescription);
    }
    sql.chop(2);

    if (!cur.exec(sql))
        throw SqlInsertionIntoTableError(cur.lastError());
}

void Database::createProducts()
{
    QString sql =
        "CREATE TABLE IF NOT EXISTS products (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tname TEXT UNIQUE NOT NULL,\n"
            "\tcodeEan TEXT UNIQUE,\n"
            "\tquantity REAL NOT NULL CHECK (quantity >= 0),\n"
            "\tunitCode TEXT NOT NULL,\n"
            "\tFOREIGN KEY (unitCode) REFERENCES units(code)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)";
    
    QSqlQuery cur(this->db);
    if (!cur.exec(sql))
        throw SqlCreationTableError(cur.lastError());
}

void Database::createTrash()
{
    QString sql =
        "CREATE TABLE IF NOT EXISTS trash (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tname TEXT NOT NULL,\n"
            "\tcodeEan TEXT,\n"
            "\tquantity REAL NOT NULL,\n"
            "\tunitCode TEXT NOT NULL,\n"
            "\tFOREIGN KEY (unitCode) REFERENCES units(code)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)";
    
    QSqlQuery cur(this->db);
    if (!cur.exec(sql))
        throw SqlCreationTableError(cur.lastError());
}

void Database::transaction()
{
    if (!this->db.transaction())
        throw SqlBeginTransactionError(this->db.lastError());
}

void Database::commit()
{
    if (!this->db.commit())
        throw SqlCommitTransactionError(this->db.lastError());
}

ScopeExit Database::makeRollbackGuard()
{
    return ScopeExit([&db = this->db]()->void{ db.rollback(); });
}
