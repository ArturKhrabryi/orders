#pragma once

#include <QMainWindow>
#include "Database.hpp"


class OrderTableView;
class ProductFormWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
	Q_OBJECT;

public:
	explicit MainWindow(QWidget* parent = nullptr);

private:
    void onAddProduct() noexcept;
    void onConvert() noexcept;
    void onClearAll() noexcept;
    void onGenerateBarcode() noexcept;
    void onDeleteSelected() noexcept;
    void onCalculateMargin() noexcept;

    Database db;

    QWidget* central;
    OrderTableView* view;
    ProductFormWidget* forms;

    QPushButton* addButton;
    QPushButton* convertButton;
    QPushButton* clearButton;
    QPushButton* barcodeButton;
    QPushButton* marginButton;

    void buildUi();
    void createActions();
    void refreshModel();
};
