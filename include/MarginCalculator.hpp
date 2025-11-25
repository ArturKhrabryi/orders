#pragma once

#include <QDialog>
#include <QDoubleSpinBox>


class MarginCalculator final : public QDialog
{
    Q_OBJECT
public:
    explicit MarginCalculator(QWidget* parent = nullptr);

private:
    enum class FieldChanged
    {
        NetPurchasePrice,
        VatPercentage,
        GrossPurchasePrice,
        MarginPercentage,
        NetSellingPrice,
        GrossSellingPrice
    };

    struct MarginParams
    {
        double netPurchasePrice = 0.0;
        double vatPercentage = 0.0;
        double grossPurchasePrice = 0.0;
        double marginPercentage = 0.0;
        double netSellingPrice = 0.0;
        double grossSellingPrice = 0.0;
    };

    QDoubleSpinBox* netPurchasePrice;
    QDoubleSpinBox* vatPercentage;
    QDoubleSpinBox* grossPurchasePrice;
    QDoubleSpinBox* marginPercentage;
    QDoubleSpinBox* netSellingPrice;
    QDoubleSpinBox* grossSellingPrice;

    void createForms() noexcept;


    MarginParams recalculate(FieldChanged FieldChanged) const noexcept;

    void onNetPurchasePriceChanged() noexcept;
    void onVatChanged() noexcept;
    void onGrossPurchasePriceChanged() noexcept;
    void onMarginChanged() noexcept;
    void onNetSellingPriceChanged() noexcept;
    void onGrossSellingPriceChanged() noexcept;

    void connectSygnals() noexcept;
};

