#include <stdexcept>
#include <QSqlError>
#include <QCoreApplication>


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
        SqlError(QCoreApplication::translate("SqlOpenDatabaseError", "Cannot open database"), sqlError)
    {}
};

struct SqlEnableForeignKeysError : SqlError
{
    SqlEnableForeignKeysError(const QSqlError& sqlError) : 
        SqlError(QCoreApplication::translate("SqlEnableForeignKeysError", "Cannot enable foreign keys"), sqlError)
    {}
};

struct SqlBeginTransactionError : SqlError
{
    SqlBeginTransactionError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlBeginTransactionError", "Cannot start sql transaction"), sqlError)
    {}
};

struct SqlCommitTransactionError : SqlError
{
    SqlCommitTransactionError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlBeginTransactionError", "Cannot commit sql transaction"), sqlError)
    {}
};

struct SqlCreationTableError : SqlError
{
    SqlCreationTableError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlCreationTableError", "Cannot create table"), sqlError)
    {}
};

struct SqlInsertionIntoTableError : SqlError
{
    SqlInsertionIntoTableError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlInsertionIntoTableError", "Cannot insert into table"), sqlError)
    {}
};

struct SqlFetchProductError: SqlError
{
    SqlFetchProductError (const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlFetchProductError", "Cannot fetch products"), sqlError)
    {}
};

struct SqlAddProductError : SqlError
{
    SqlAddProductError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlAddProductError", "Cannot add product"), sqlError)
    {}
};

struct SqlMoveToTrashError : SqlError
{
    SqlMoveToTrashError(const QSqlError& sqlError) :
        SqlError(QCoreApplication::translate("SqlMoveToTrashError", "Cannot move products to trash"), sqlError)
    {}
};
