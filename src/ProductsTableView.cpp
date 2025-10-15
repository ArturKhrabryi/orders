#include "ProductsTableView.hpp"
#include <QHeaderView>
#include "DatabaseProductsModel.hpp"


ProductsTableView::ProductsTableView(QWidget* parent) noexcept : QTableView(parent)
{
    this->setSelectionBehavior(QAbstractItemView::SelectItems);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    //this->verticalHeader()->setVisible(false);
    this->setAlternatingRowColors(true);
}

void ProductsTableView::attachModel(QAbstractItemModel* model) noexcept
{
    this->setModel(model);
    this->setColumnHidden(0, true);

    auto* header = this->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    this->setItemDelegateForColumn(2, new DatabaseProductsModel::EanDelegate(this));
    this->resizeColumnsToContents(); 
}

