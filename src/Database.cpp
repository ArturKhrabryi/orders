#include "Database.hpp"

#include <QString>
#include <QSqlRecord>
#include <QSqlError>
#include <qsqlquery.h>
#include "Product.hpp"


Database::Database() : db(QSqlDatabase::addDatabase("QSQLITE"))
{
    this->db.setDatabaseName("zamówienie.db");
    if (!this->db.open())
        throw SqlOpenDatabaseError(this->db.lastError());
    
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    QSqlQuery cur(this->db);
    if (!cur.exec("PRAGMA foreign_keys = ON"))
        throw SqlEnableForeignKeysError(cur.lastError());

    this->createUnits();
    this->createProducts();
    this->createTrash();

    this->commit();

    rollbackGuard.release();
}

Database::~Database()
{
    if (this->db.isOpen())
        this->db.close();

    auto name = this->db.connectionName();
    this->db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

std::vector<Product> Database::fetchProducts() const
{
    QSqlQuery cur(this->db); 
    cur.setForwardOnly(true);

    const QString sql = "SELECT name, codeEan, quantity, unitCode FROM products ORDER BY id ASC";
    if (!cur.exec(sql))
        throw SqlFetchProductsError(cur.lastError());

    const auto rec = cur.record();
    auto iName = rec.indexOf("name");
    auto iCodeEan = rec.indexOf("codeEan");
    auto iQuantity = rec.indexOf("quantity");
    auto iUnitcode = rec.indexOf("unitCode");

    std::vector<Product> products;
    products.reserve(64);
    while (cur.next())
    {
        Product product;
        product.name = cur.value(iName).toString();
        std::optional<CodeEan> codeEan = std::nullopt;
        auto codeEanValue = cur.value(iCodeEan);
        if (codeEanValue.isNull() || codeEanValue.toString().isEmpty())
            codeEan = cur.value(iCodeEan).toString();
        product.codeEan = std::move(codeEan);
        product.quantity = cur.value(iQuantity).toFloat();
        product.unitCode = cur.value(iUnitcode).toString();

        products.push_back(std::move(product));
    }

    return products;
}

void Database::addProduct(const Product& product)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();


    QString sql =  "INSERT INTO products (name, codeEan, quantity, unitCode) VALUES (?, ?, ?, ?)";
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

    sql =
        "INSERT OR IGNORE INTO units (code, name) VALUES\n"
            "\t('kpl', 'komplet'),\n"
            "\t('op', 'opakowanie'),\n"
            "\t('pal', 'paleta'),\n"
            "\t('szt', 'sztuka'),\n"
            "\t('t', 'tona'),\n"
            "\t('m2', 'metr kwadratowy'),\n"
            "\t('rol', 'rolka')";

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

    if (!cur.exec("CREATE INDEX IF NOT EXISTS idx_products_code_ean ON products(codeEan)"))
        throw SqlCreationIndexError(cur.lastError());
}

void Database::createTrash()
{
    QString sql =
        "CREATE TABLE IF NOT EXISTS trash (\n"
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

    if (!cur.exec("CREATE INDEX IF NOT EXISTS idx_trash_code_ean ON trash(codeEan)"))
        throw SqlCreationIndexError(cur.lastError());
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
    return ScopeExit([&db=db]()->void{ db.rollback(); });
}
