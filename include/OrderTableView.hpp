#pragma once

#include <QTableView>


class OrderTableView : public QTableView
{
    Q_OBJECT
public:
    explicit OrderTableView(QWidget* parent = nullptr) noexcept;
    void attachModel(QAbstractItemModel* model) noexcept;
};
