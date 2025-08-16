#include "Database.hpp"

#include <QString>
#include <QSqlRecord>
#include <stdexcept>
#include <QSqlError>
#include "Product.hpp"


Database::Database() : db(QSqlDatabase::addDatabase("QSQLITE"))
{
    this->db.setDatabaseName("zamówienie.db");
    if (!this->db.open())
        throw std::runtime_error("Cannot open database.");

    QSqlQuery cur(this->db);
    if (!cur.exec("PRAGMA foreign_keys = ON"))
        throw std::runtime_error("Cannot enable foreign keys: " + cur.lastError().text().toStdString());

    this->transaction();

    auto rollbackGuard = this->makeRollbackGuard();

    QString sql =
        "CREATE TABLE IF NOT EXISTS units (\n"
            "\tcode TEXT PRIMARY KEY,\n"
            "\tname TEXT NOT NULL\n"
            ") WITHOUT ROWID";

    if (!cur.exec(sql))
        throw std::runtime_error("Cannot create table units: " + cur.lastError().text().toStdString());

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
        throw std::runtime_error("Cannot insert values into units: " + cur.lastError().text().toStdString());

    sql =
        "CREATE TABLE IF NOT EXISTS products (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tname TEXT UNIQUE NOT NULL,\n"
            "\tcodeEan TEXT UNIQUE,\n"
            "\tquantity REAL NOT NULL CHECK (quantity >= 0),\n"
            "\tunitCode TEXT NOT NULL,\n"
            "\tFOREIGN KEY (unitCode) REFERENCES units(code)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)";
    
    if (!cur.exec(sql))
        throw std::runtime_error("Cannot create table products: " + cur.lastError().text().toStdString());

    if (!cur.exec("CREATE INDEX IF NOT EXISTS idx_products_code_ean ON products(codeEan)"))
        throw std::runtime_error("Cannot create index: " + cur.lastError().text().toStdString());

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
        throw std::runtime_error("Cannot fetch products: " + cur.lastError().text().toStdString());

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
        throw std::runtime_error("Cannot add product: " + cur.lastError().text().toStdString());

    this->commit();
    rollbackGuard.release();
}

void Database::transaction()
{
    if (!db.transaction())
        throw std::runtime_error("Cannot start transaction.");
}

void Database::commit()
{
    if (!db.commit())
        throw std::runtime_error("Commit failed.");
}

ScopeExit Database::makeRollbackGuard()
{
    return ScopeExit([&db=db]()->void{ db.rollback(); });
}
