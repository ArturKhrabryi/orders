#include "MarginCalculator.hpp"
#include <QVBoxLayout>
#include <QLabel>


QDoubleValidator::State MarginValidator::validate(QString& str, int& pos) const
{
    if (str.isEmpty())
        return Acceptable;

    if (bottom() < 0 && str == "-")
        return Acceptable;

    bool ok = false;
    double value = locale().toDouble(str, &ok);
    if (!ok)
        return Invalid;

    if (value < bottom() || value > top())
        return Invalid;

    return Acceptable;
}

MarginCalculator::MarginCalculator(QWidget* parent) :
    QDialog(parent),
    netPurchasePrice(new QLineEdit(this)),
    vatPercentage(new QLineEdit(this)),
    grossPurchasePrice(new QLineEdit(this)),
    marginPercentage(new QLineEdit(this)),
    netSellingPrice(new QLineEdit(this)),
    grossSellingPrice(new QLineEdit(this)),
    priceValidator(new MarginValidator(0, 1000000000, 2, this)),
    percentageValidator(new MarginValidator(-100, 100, 2, this))
{
    this->createForms();
    this->setValidators();
}

void MarginCalculator::createForms() noexcept
{
    auto* layout = new QVBoxLayout(this);
    auto makeForm = [&](const QString& labelText, const QString& placeholder, QLineEdit* form) -> void
    {
        auto label = new QLabel(labelText, this);
        label->setBuddy(form);
        layout->addWidget(label);
        layout->addWidget(form);
        form->setPlaceholderText(placeholder);
    };

    makeForm(tr("&Net purchase price"), tr("Net purchase price"), this->netPurchasePrice);
    makeForm(tr("&VAT"), tr("VAT"), this->vatPercentage);
    this->vatPercentage->setText("23");
    makeForm(tr("&Gross purchase price"), tr("Gross purchase price"), this->grossPurchasePrice);
    makeForm(tr("&Margin percentage"), tr("Margin percentage"), this->marginPercentage);
    makeForm(tr("N&et selling price"), tr("Net selling price"), this->netSellingPrice);
    makeForm(tr("G&ross selling price"), tr("Gross selling price"), this->grossSellingPrice);
}

void MarginCalculator::setValidators() noexcept
{
    this->netPurchasePrice->setValidator(this->priceValidator);
    this->vatPercentage->setValidator(this->percentageValidator);
    this->grossPurchasePrice->setValidator(this->priceValidator);
    this->marginPercentage->setValidator(this->percentageValidator);
    this->netSellingPrice->setValidator(this->priceValidator);
    this->grossSellingPrice->setValidator(this->priceValidator);
}
