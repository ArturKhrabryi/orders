#pragma once

#include <QStyledItemDelegate>
#include <QAbstractTableModel>
#include <QVariant>
#include <QSqlDatabase>
#include <QVector>


class Database;

class CodeEanDelegate : public QStyledItemDelegate
{
    Q_OBJECT
        
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept override;
};

class QuantityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
        
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const noexcept override;
};

class OrderTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class ColumnName : int
    {
        Id = 0,
        ProductId,
        Name,
        CodeEan,
        Quantity,
        UnitCode,

        Count
    };

    struct Row
    {
        int id;
        int productId;
        QString name;
        QString codeEan;
        double quantity;
        QString unitCode;
    };

    static QString realColumnName(ColumnName columnName);

    explicit OrderTableModel(Database* database, QObject* parent = nullptr);
    void select();
    int rowCount(const QModelIndex& parent = QModelIndex()) const noexcept override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const noexcept override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const noexcept override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const noexcept override;
    Qt::ItemFlags flags(const QModelIndex& index) const noexcept override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) noexcept override;

private:
    Database* database;
    QVector<Row> rows;
};
