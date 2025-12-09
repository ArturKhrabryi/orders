#include "MarginCalculator.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <limits>
#include <QSignalBlocker>


MarginCalculator::MarginCalculator(QWidget* parent) :
    QDialog(parent),
    netPurchasePrice(new QDoubleSpinBox(this)),
    vatPercentage(new QDoubleSpinBox(this)),
    grossPurchasePrice(new QDoubleSpinBox(this)),
    marginPercentage(new QDoubleSpinBox(this)),
    netSellingPrice(new QDoubleSpinBox(this)),
    grossSellingPrice(new QDoubleSpinBox(this))
{
    this->createForms();
    this->connectSygnals();
}

void MarginCalculator::createForms() noexcept
{
    auto* layout = new QVBoxLayout(this);
    auto makeForm = [&](const QString& labelText, double defaultValue, QDoubleSpinBox* form) -> void
    {
        auto label = new QLabel(labelText, this);
        label->setBuddy(form);
        layout->addWidget(label);
        layout->addWidget(form);
        form->setValue(defaultValue);
    };

    makeForm(tr("&Net purchase price"), 0.0, this->netPurchasePrice);
    makeForm(tr("&VAT"), 23.0, this->vatPercentage);
    makeForm(tr("&Gross purchase price"), 0.0, this->grossPurchasePrice);
    makeForm(tr("&Margin percentage"), 30.0, this->marginPercentage);
    makeForm(tr("N&et selling price"), 0.0, this->netSellingPrice);
    makeForm(tr("G&ross selling price"), 0.0, this->grossSellingPrice);

    this->netPurchasePrice->setMaximum(std::numeric_limits<double>::max());
    this->vatPercentage->setMaximum(std::numeric_limits<double>::max());
    this->grossPurchasePrice->setMaximum(std::numeric_limits<double>::max());
    this->marginPercentage->setRange(-99.99, 99.99);
    this->netSellingPrice->setMaximum(std::numeric_limits<double>::max());
    this->grossSellingPrice->setMaximum(std::numeric_limits<double>::max());
}

void MarginCalculator::onNetPurchasePriceChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::NetPurchasePrice);

    QSignalBlocker b1(this->grossPurchasePrice);
    QSignalBlocker b2(this->netSellingPrice);
    QSignalBlocker b3(this->grossSellingPrice);

    this->grossPurchasePrice->setValue(marginParams.grossPurchasePrice);
    this->netSellingPrice->setValue(marginParams.netSellingPrice);
    this->grossSellingPrice->setValue(marginParams.grossSellingPrice);
}

void MarginCalculator::onVatChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::VatPercentage);

    QSignalBlocker b1(this->grossPurchasePrice);
    QSignalBlocker b3(this->grossSellingPrice);

    this->grossPurchasePrice->setValue(marginParams.grossPurchasePrice);
    this->grossSellingPrice->setValue(marginParams.grossSellingPrice);
}

void MarginCalculator::onGrossPurchasePriceChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::GrossPurchasePrice);

    QSignalBlocker b1(this->vatPercentage);
    QSignalBlocker b2(this->grossSellingPrice);

    this->vatPercentage->setValue(marginParams.vatPercentage);
    this->grossSellingPrice->setValue(marginParams.grossSellingPrice);
}

void MarginCalculator::onMarginChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::MarginPercentage);

    QSignalBlocker b1(this->netSellingPrice);
    QSignalBlocker b2(this->grossSellingPrice);

    this->netSellingPrice->setValue(marginParams.netSellingPrice);
    this->grossSellingPrice->setValue(marginParams.grossSellingPrice);
}

void MarginCalculator::onNetSellingPriceChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::NetSellingPrice);

    QSignalBlocker b1(this->marginPercentage);
    QSignalBlocker b2(this->grossSellingPrice);

    this->marginPercentage->setValue(marginParams.marginPercentage);
    this->grossSellingPrice->setValue(marginParams.grossSellingPrice);
}

void MarginCalculator::onGrossSellingPriceChanged() noexcept
{
    auto marginParams = this->recalculate(FieldChanged::GrossSellingPrice);

    QSignalBlocker b1(this->marginPercentage);
    QSignalBlocker b2(this->netSellingPrice);

    this->marginPercentage->setValue(marginParams.marginPercentage);
    this->netSellingPrice->setValue(marginParams.netSellingPrice);
}

auto MarginCalculator::recalculate(FieldChanged fieldChanged) const noexcept -> MarginParams
{
    double netPurchasePrice = 0.0;
    double vatPercentage = 0.0;
    double grossPurchasePrice = 0.0;
    double marginPercentage = 0.0;
    double netSellingPrice = 0.0;
    double grossSellingPrice = 0.0;

    netPurchasePrice = this->netPurchasePrice->value();
    
    switch (fieldChanged)
    {
        {
        case FieldChanged::NetPurchasePrice:
            vatPercentage = this->vatPercentage->value(); 
            double vat = vatPercentage / 100.0 + 1.0;
            grossPurchasePrice = netPurchasePrice * vat;
            marginPercentage = this->marginPercentage->value();
            double margin = marginPercentage / 100.0;
            margin = 1.0 - margin;
            netSellingPrice = netPurchasePrice / margin;
            grossSellingPrice = netSellingPrice * vat;
            break;
        }
        {
        case FieldChanged::VatPercentage:
            vatPercentage = this->vatPercentage->value(); 
            double vat = vatPercentage / 100.0 + 1.0;
            grossPurchasePrice = netPurchasePrice * vat;
            marginPercentage = this->marginPercentage->value();
            netSellingPrice = this->netSellingPrice->value();
            grossSellingPrice = netSellingPrice * vat;

            break;
        }
        {
        case FieldChanged::GrossPurchasePrice:
            grossPurchasePrice = this->grossPurchasePrice->value();
            double vat = grossPurchasePrice / netPurchasePrice;
            vatPercentage = vat - 1.0;
            vatPercentage *= 100.0;
            marginPercentage = this->marginPercentage->value();
            netSellingPrice = this->netSellingPrice->value();
            grossSellingPrice = netSellingPrice * vat;
            break;
        }
        {
        case FieldChanged::MarginPercentage:
            vatPercentage = this->vatPercentage->value(); 
            grossPurchasePrice = this->grossPurchasePrice->value();
            marginPercentage = this->marginPercentage->value();
            double margin = marginPercentage / 100.0;
            margin = 1.0 - margin;
            netSellingPrice = netPurchasePrice / margin;
            double vat = vatPercentage / 100.0 + 1.0;
            grossSellingPrice = netSellingPrice * vat;
            break;
        }
        {
        case FieldChanged::NetSellingPrice:
            vatPercentage = this->vatPercentage->value(); 
            grossPurchasePrice = this->grossPurchasePrice->value();
            netSellingPrice = this->netSellingPrice->value();
            double margin = netPurchasePrice / netSellingPrice;
            margin = 1.0 - margin;
            marginPercentage = margin * 100.0;
            double vat = vatPercentage / 100.0 + 1.0;
            grossSellingPrice = netSellingPrice * vat;
            break;
        }
        {
        case FieldChanged::GrossSellingPrice:
            vatPercentage = this->vatPercentage->value(); 
            grossPurchasePrice = this->grossPurchasePrice->value();
            grossSellingPrice = this->grossSellingPrice->value();
            double vat = vatPercentage / 100.0 + 1.0;
            netSellingPrice = grossSellingPrice / vat;
            double margin = netPurchasePrice / netSellingPrice;
            margin = 1.0 - margin;
            marginPercentage = margin * 100.0;
            break;
        }
    }

    return { netPurchasePrice, vatPercentage, grossPurchasePrice, marginPercentage, netSellingPrice, grossSellingPrice };
}

void MarginCalculator::connectSygnals() noexcept
{
   this->connect(this->netPurchasePrice,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onNetPurchasePriceChanged); 

   this->connect(this->vatPercentage,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onVatChanged); 

   this->connect(this->grossPurchasePrice,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onGrossPurchasePriceChanged); 
   
   this->connect(this->marginPercentage,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onMarginChanged); 

   this->connect(this->netSellingPrice,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onNetSellingPriceChanged); 

   this->connect(this->grossSellingPrice,
           qOverload<double>(&QDoubleSpinBox::valueChanged),
           this, &MarginCalculator::onGrossSellingPriceChanged); 
}
