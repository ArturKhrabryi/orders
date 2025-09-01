#pragma once

#include <QMainWindow>
#include <optional>
#include "Product.hpp"
#include "CodeEan.hpp"
#include "Database.hpp"


class QWidget;
class QTableView;
class QLineEdit;
class QPushButton;

class MainWindow : public QMainWindow
{
	Q_OBJECT;

public:
	explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void handleEnterButton() noexcept;
    void handleConvertButton() const noexcept;
    void handleClearButton() noexcept;
    void handleBarcodeGenerationButton() noexcept;
    void handleDeleteItem() noexcept;

private:
    QWidget* central;
    QTableView* view;
    QLineEdit* nameForm;
    QLineEdit* codeEanForm;
    QLineEdit* quantityForm;
    QLineEdit* unitCodeForm;
    QPushButton* enterButton;
    QPushButton* convertButton;
    QPushButton* clearButton;
    QPushButton* barcodeGenerationButton;

    Database db;

    void setPlaceholders() noexcept;
    void clearForms() noexcept;
    void updateView();

    Product getProductFromForms() const;
    QString getNameFromForm() const;
    static QString normalize(const QString& input) noexcept;
    std::optional<CodeEan> getCodeEanFromForm() const;
    float getQuantityFromForm() const;
    QString getUnitCodeFromForm() const;

    void createDeleteAction() noexcept;
};
