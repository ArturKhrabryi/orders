#pragma once

#include <QWidget>
#include "Product.hpp"


class QLineEdit;

class ProductFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProductFormWidget(QWidget* parent = nullptr);
    Product getProduct() const;
    void clear() noexcept;
    void focusNameForm() noexcept;

protected:
    QLineEdit* nameForm;
    QLineEdit* codeEanForm;
    QLineEdit* quantityForm;
    QLineEdit* unitCodeForm;

    static QString normalizeWords(const QString& str) noexcept;
};
