#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSqlDatabase>
#include "Product.hpp"


class QLineEdit;

class ProductFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProductFormWidget(QWidget* parent = nullptr, const QSqlDatabase& db = QSqlDatabase());
    ProductFormData getProductFormData() const;
    void clear() noexcept;
    void focusNameForm() noexcept;

protected:
    QLineEdit* nameForm;
    QLineEdit* codeEanForm;
    QDoubleSpinBox* quantityForm;
    QComboBox* unitCodeForm;

    static QString normalizeWords(const QString& str) noexcept;
};
