#include "OrderTableModel.hpp"
#include "Database.hpp"
#include "CodeEan.hpp"
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlError>
#include <exception>
#include <stdexcept>
#include <QSqlRecord>
#include <QDoubleSpinBox>
#include <limits>


void CodeEanDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept
{
    auto *ed = qobject_cast<QLineEdit*>(editor);
    if (!ed) return;

    const QString text = ed->text();
    try
    {
        CodeEan ean(text);
        model->setData(index, text);
    }
    catch (const std::exception& ex)
    {
        QMessageBox::warning(editor, tr("Code EAN error"), ex.what());
        const_cast<QLineEdit*>(ed)->setFocus();
    }
}

QWidget* QuantityDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const noexcept
{
    auto* editor = new QDoubleSpinBox(parent);
    editor->setDecimals(3);
    editor->setMinimum(0.0);
    editor->setMaximum(std::numeric_limits<double>::max());
    editor->setSingleStep(1.0);

    return editor;
}

QString OrderTableModel::realColumnName(ColumnName columnName)
{
    switch (columnName)
    {
    case ColumnName::Id:
        [[fallthrough]];
    case ColumnName::ProductId:
        return QStringLiteral("id");

    case ColumnName::Name:
        return QStringLiteral("name");

    case ColumnName::CodeEan:
        return QStringLiteral("codeEan");

    case ColumnName::Quantity:
        return QStringLiteral("quantity");

    case ColumnName::UnitCode:
        return QStringLiteral("unitCode");


    case ColumnName::Count:
        break;
    }

    throw std::runtime_error(tr("Undefined column name").toStdString());
}

OrderTableModel::OrderTableModel(Database* database, QObject* parent) :
    QAbstractTableModel(parent),
    database(database)
{
    if (!database)
        throw std::runtime_error(tr("Database does not exist").toStdString());

    this->select();
}

void OrderTableModel::select()
{
    QSqlDatabase db = this->database->getDb();
    QSqlQuery cur(db);
    cur.setForwardOnly(true);
    const QString sql = QStringLiteral(
        "SELECT orderLines.id, products.id, products.name, products.codeEan, orderLines.quantity, products.unitCode\n"
        "FROM orderLines\n"
        "JOIN products ON products.id = orderLines.productId\n"
        "ORDER BY orderLines.id ASC"        
    );
    
    if (!cur.exec(sql))
    {
        const auto err = cur.lastError();
        throw std::runtime_error(err.text().toStdString());
    }

    QVector<Row> newRows;
    constexpr int initialSize = 128;
    newRows.reserve(initialSize);

    while (cur.next())
    {
        newRows.push_back({
            cur.value(static_cast<int>(ColumnName::Id)).toInt(),
            cur.value(static_cast<int>(ColumnName::ProductId)).toInt(),
            cur.value(static_cast<int>(ColumnName::Name)).toString(),
            cur.value(static_cast<int>(ColumnName::CodeEan)).toString(),
            cur.value(static_cast<int>(ColumnName::Quantity)).toDouble(),
            cur.value(static_cast<int>(ColumnName::UnitCode)).toString()
        });
    }

    this->beginResetModel();
    this->rows = std::move(newRows);
    this->endResetModel();
}

int OrderTableModel::rowCount(const QModelIndex& parent) const noexcept
{
    if (parent.isValid())
        return 0;

    return this->rows.size();
}

int OrderTableModel::columnCount(const QModelIndex& parent) const noexcept
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(ColumnName::Count);
}

QVariant OrderTableModel::headerData(int section, Qt::Orientation orientation, int role) const noexcept
{
    static const QStringList headers = {
        tr("Id"),
        tr("Product id"),
        tr("Name"),
        tr("Code ean"),
        tr("Quantity"),
        tr("Unit")
    };

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section >= 0 && section < headers.size())
            return headers[section];
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant OrderTableModel::data(const QModelIndex& index, int role) const noexcept
{
    if (!index.isValid())
        return {};

    const int rowId = index.row();

    if (rowId < 0 || rowId >= this->rows.size())
        return {};

    const int columnId = index.column();

    if (columnId < 0 || columnId >= static_cast<int>(ColumnName::Count))
        return {};

    const auto& row = this->rows[rowId];
    const auto column = static_cast<ColumnName>(columnId); 

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (column)
        {
            case ColumnName::Id: return row.id;
            case ColumnName::ProductId: return row.productId;
            case ColumnName::Name: return row.name;
            case ColumnName::CodeEan: return row.codeEan;
            case ColumnName::Quantity: return row.quantity;
            case ColumnName::UnitCode: return row.unitCode;
            case ColumnName::Count: break;
        }
    }

    if (role == Qt::TextAlignmentRole)
    {
        if (column == ColumnName::Name)
            return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);

        return Qt::AlignCenter;
    }

    return {};
}

Qt::ItemFlags OrderTableModel::flags(const QModelIndex& index) const noexcept
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    const int columnId = index.column();
    if (columnId < 0 || columnId >= static_cast<int>(ColumnName::Count))
        return Qt::NoItemFlags;

    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
    
    const auto column = static_cast<ColumnName>(columnId);
    switch (column)
    {
        case ColumnName::Id:
            [[fallthrough]];
        case ColumnName::ProductId:
            return flags;

        default:
            flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool OrderTableModel::setData(const QModelIndex& index, const QVariant& value, int role) noexcept
{
    if (role != Qt::EditRole || !index.isValid())
        return false;

    const int rowId = index.row();
    if (rowId < 0 || rowId >= this->rows.size())
        return false;

    const auto columnId = index.column();
    if (columnId <= 0 || columnId >= static_cast<int>(ColumnName::Count))
        return false;

    const auto column = static_cast<ColumnName>(index.column());

    auto& row = this->rows[rowId];
    try
    {
        this->database->updateColumn(row, column, value);
    }
    catch (...)
    {
        return false;
    }

    switch (column)
    {
        case ColumnName::Id:
            [[fallthrough]];
        case ColumnName::ProductId:
            [[fallthrough]];
        case ColumnName::Count:
            return false;

        case ColumnName::Name:
            row.name = value.toString();
            break;

        case ColumnName::CodeEan:
            row.codeEan = value.toString();
            break;

        case ColumnName::Quantity:
            row.quantity = value.toDouble();
            break;
        
        case ColumnName::UnitCode:
            row.unitCode = value.toString();
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return true;
}
