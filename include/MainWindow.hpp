#pragma once

#include <QMainWindow>
#include "Database.hpp"


class ProductsTableView;
class ProductFormWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
	Q_OBJECT;

public:
	explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAddProduct() noexcept;
    void onConvert() noexcept;
    void onClearAll() noexcept;
    void onGenerateBarcode() noexcept;
    void onDeleteSelected() noexcept;

private:
    QWidget* central;
    ProductsTableView* view;
    ProductFormWidget* forms;

    QPushButton* addButton;
    QPushButton* convertButton;
    QPushButton* clearButton;
    QPushButton* barcodeButton;

    Database db;

    void buildUi();
    void createActions();
    void refreshModel();
};
