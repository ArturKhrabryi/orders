#pragma once

#include <QMainWindow>
#include <optional>
#include "Database.hpp"
#include "Product.hpp"
#include "CodeEan.hpp"


class QWidget;
class QTextEdit;
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
    void handleDeleteButton() noexcept;
    void handleClearButton() noexcept;
    void handleBarcodeGenerationButton() const noexcept;

private:
    QWidget* central = nullptr;
    QTextEdit* view = nullptr;
    QLineEdit* nameForm = nullptr;
    QLineEdit* codeEanForm = nullptr;
    QLineEdit* quantityForm = nullptr;
    QLineEdit* unitCodeForm = nullptr;
    QPushButton* enterButton = nullptr;
    QPushButton* convertButton = nullptr;
    QPushButton* deleteButton = nullptr;
    QPushButton* clearButton = nullptr;
    QPushButton* barcodeGenerationButton = nullptr;

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
};
