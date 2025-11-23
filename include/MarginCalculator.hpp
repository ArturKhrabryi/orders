#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QDoubleValidator>


class MarginValidator : public QDoubleValidator
{
public:
    using QDoubleValidator::QDoubleValidator;
    State validate(QString& str, int& pos) const override;
};

class MarginCalculator final : public QDialog
{
    Q_OBJECT
public:
    explicit MarginCalculator(QWidget* parent = nullptr);

private:
    QLineEdit* netPurchasePrice;
    QLineEdit* vatPercentage;
    QLineEdit* grossPurchasePrice;
    QLineEdit* marginPercentage;
    QLineEdit* netSellingPrice;
    QLineEdit* grossSellingPrice;

    MarginValidator* priceValidator;
    QDoubleValidator* percentageValidator;

    void createForms() noexcept;
    void setValidators() noexcept;
};

