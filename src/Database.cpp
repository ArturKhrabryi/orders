#include "Database.hpp"
#include <QString>
#include <QSqlRecord>
#include <QCoreApplication>
#include <qsqlquery.h>
#include <stdexcept>
#include <vector>
#include "Product.hpp"
#include "DatabaseError.hpp"


QSqlDatabase Database::createDb()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("order.db");
    if (!db.open())
        throw SqlOpenDatabaseError(db.lastError());

    QSqlQuery cur(db);
    if (!cur.exec("PRAGMA foreign_keys = ON"))
        throw SqlEnableForeignKeysError(cur.lastError());

    db.transaction();
    auto rollbackGuard = ScopeExit([&db]()->void{ db.rollback(); });

    createUnits(db);
    createProducts(db);
    createOrderLines(db);
    createTrash(db);

    db.commit();
    rollbackGuard.release();

    return db;
}

Database::Database() : db(createDb()), model(this)
{
}

Database::~Database()
{
    if (this->db.isOpen())
        this->db.close();
}

bool Database::exists(const CodeEan& codeEan) const
{
    throw std::runtime_error("Not implemented yet");

    return false; 
}

QVariant Database::insertOrGetProduct(const Product& product)
{
    QSqlQuery cur(this->db);
    cur.setForwardOnly(true);

    if (product.codeEan)
    {
        if (product.name.isEmpty())
        {
            cur.prepare(QStringLiteral(
                "SELECT id FROM products "
                "WHERE codeEan = ?"
            ));

            cur.addBindValue(product.codeEan->getValue());
        }
        else
        {
            cur.prepare(QStringLiteral(
                "INSERT INTO products (name, codeEan, unitCode)\n"
                "VALUES (?, ?, ?)\n"
                "ON CONFLICT(codeEan) DO UPDATE SET id = id\n"
                "RETURNING id"
            ));

            cur.addBindValue(product.name);
            cur.addBindValue(product.codeEan->getValue());
            cur.addBindValue(product.unitCode);
        }
    }
    else
    {
        cur.prepare(QStringLiteral(
            "INSERT INTO products (name, codeEan, unitCode)\n"
            "VALUES (?, NULL, ?)\n"
            "ON CONFLICT(name) DO UPDATE SET id = id\n"
            "RETURNING id"
        ));

        cur.addBindValue(product.name);
        cur.addBindValue(product.unitCode);
    }

    if (!cur.exec())
        throw SqlAddProductError(cur.lastError());

    if (!cur.next())
        throw SqlError(QCoreApplication::translate("Database", "'RETURNING' did not return id"));

    return cur.value(0);
}

void Database::addOrderLine(const ProductFormData& productFormData)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    auto productId = this->insertOrGetProduct({ productFormData.name, productFormData.codeEan, productFormData.unitCode });

    QSqlQuery cur(this->db);
    cur.setForwardOnly(true);
    cur.prepare(QStringLiteral(
        "INSERT INTO orderLines (productId, quantity) "
        "VALUES (?, ?)"
    ));
    cur.addBindValue(productId);
    cur.addBindValue(productFormData.quantity);

    if (!cur.exec())
        throw SqlAddOrderLineError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::updateColumn(const OrderTableModel::Row& row , OrderTableModel::ColumnName columnName, const QVariant& data)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    bool isOrdersTable = columnName == OrderTableModel::ColumnName::Quantity;
    QString tableName = isOrdersTable ? "orderLines" : "products";
    auto columnNameStr = OrderTableModel::realColumnName(columnName);
    auto id = isOrdersTable ? row.id : row.productId;
    
    QString sql = QStringLiteral(
        "UPDATE %1\n"
        "SET %2 = ?\n"
        "WHERE id = ?"
    ).arg(tableName, columnNameStr);

    QSqlQuery cur(this->db);
    cur.prepare(sql);
    cur.addBindValue(data); 
    cur.addBindValue(id);

    if (!cur.exec())
        throw SqlUpdateColumnError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::moveOrderLineToTrash(int id)
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    QString sql = "INSERT INTO trash (productId, quantity) SELECT productId, quantity FROM orderLines WHERE id=?";
    QSqlQuery cur(this->db);
    cur.prepare(sql);
    cur.addBindValue(id);

    if (!cur.exec())
        throw SqlMoveToTrashError(cur.lastError());

    sql = "DELETE FROM orderLines where id=?";
    cur.prepare(sql);
    cur.addBindValue(id);

    if (!cur.exec())
        throw SqlMoveToTrashError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::moveOrderLinesToTrash()
{
    this->transaction();
    auto rollbackGuard = this->makeRollbackGuard();

    QString sql = "INSERT INTO trash (productId, quantity) SELECT productId, quantity FROM orderLines";
    QSqlQuery cur(this->db);
    if (!cur.exec(sql))
        throw SqlMoveToTrashError(cur.lastError());

    sql = "DELETE FROM orderLines";
    if (!cur.exec(sql))
        throw SqlMoveToTrashError(cur.lastError());

    this->commit();
    rollbackGuard.release();
}

void Database::createUnits(QSqlDatabase& db)
{
    QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS units (\n"
            "\tcode TEXT PRIMARY KEY,\n"
            "\tname TEXT NOT NULL\n"
            ") WITHOUT ROWID"
    );

    QSqlQuery cur(db);
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

void Database::createProducts(QSqlDatabase& db)
{
    QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS products (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tname TEXT UNIQUE NOT NULL,\n"
            "\tcodeEan TEXT UNIQUE,\n"
            "\tunitCode TEXT NOT NULL,\n"
            "\tFOREIGN KEY (unitCode) REFERENCES units(code)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)"
    );

    QSqlQuery cur(db);
    if (!cur.exec(sql))
        throw SqlCreationTableError(cur.lastError());
}

void Database::createOrderLines(QSqlDatabase& db)
{
    QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS orderLines (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tproductId INTEGER UNIQUE NOT NULL,\n"
            "\tquantity REAL NOT NULL CHECK (quantity >= 0),\n"
            "\tFOREIGN KEY (productId) REFERENCES products(id)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)"
    );
    
    QSqlQuery cur(db);
    if (!cur.exec(sql))
        throw SqlCreationTableError(cur.lastError());
}

void Database::createTrash(QSqlDatabase& db)
{
    QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS trash (\n"
            "\tid INTEGER PRIMARY KEY,\n"
            "\tproductId INTEGER NOT NULL,\n"
            "\tquantity REAL NOT NULL,\n"
            "\tcreated_at INTEGER NOT NULL DEFAULT(strftime('%s', 'now')),\n"
            "\tFOREIGN KEY (productId) REFERENCES products(id)\n"
            "\tON UPDATE CASCADE\n"
            "\tON DELETE RESTRICT)"
    );
    
    QSqlQuery cur(db);
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
