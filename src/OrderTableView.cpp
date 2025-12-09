#include "OrderTableView.hpp"
#include <QHeaderView>
#include "OrderTableModel.hpp"


OrderTableView::OrderTableView(QWidget* parent) noexcept : QTableView(parent)
{
    this->setSelectionBehavior(QAbstractItemView::SelectItems);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    this->setAlternatingRowColors(true);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void OrderTableView::attachModel(QAbstractItemModel* model) noexcept
{
    this->setModel(model);

    auto* header = this->horizontalHeader();
    this->setColumnHidden(static_cast<int>(OrderTableModel::ColumnName::Id), true);
    this->setColumnHidden(static_cast<int>(OrderTableModel::ColumnName::ProductId), true);
    header->setStretchLastSection(false);
    header->setSectionResizeMode(static_cast<int>(OrderTableModel::ColumnName::Name), QHeaderView::Stretch);
    header->setSectionResizeMode(static_cast<int>(OrderTableModel::ColumnName::CodeEan), QHeaderView::ResizeToContents);
    header->setSectionResizeMode(static_cast<int>(OrderTableModel::ColumnName::Quantity), QHeaderView::ResizeToContents);
    header->setSectionResizeMode(static_cast<int>(OrderTableModel::ColumnName::UnitCode), QHeaderView::ResizeToContents);
    
    this->setItemDelegateForColumn(static_cast<int>(OrderTableModel::ColumnName::CodeEan), new CodeEanDelegate(this));
    this->setItemDelegateForColumn(static_cast<int>(OrderTableModel::ColumnName::Quantity), new QuantityDelegate(this));

    this->resizeColumnsToContents(); 
}

