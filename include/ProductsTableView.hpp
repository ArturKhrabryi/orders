#pragma once

#include <QTableView>


class ProductsTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ProductsTableView(QWidget* parent = nullptr) noexcept;
    void attachModel(QAbstractItemModel* model) noexcept;
};
