#include "ProductFormWidget.hpp"
#include "CodeEan.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>


ProductFormWidget::ProductFormWidget(QWidget* parent) :
    QWidget(parent),
    nameForm(new QLineEdit(this)),
    codeEanForm(new QLineEdit(this)),
    quantityForm(new QLineEdit(this)),
    unitCodeForm(new QLineEdit(this))
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

    makeForm(tr("&Name"), tr("Name"), this->nameForm);
    makeForm(tr("&Code ean"), tr("Code ean"), this->codeEanForm);
    makeForm(tr("&Quantity"), tr("Quantity"), this->quantityForm);
    makeForm(tr("&Unit"), tr("Unit"), this->unitCodeForm);
}

QString ProductFormWidget::normalizeWords(const QString& input) noexcept
{
    auto words = input.simplified().split(' ');
    for (auto& word : words) word = word.left(1).toUpper() + word.mid(1).toLower();

    return words.join(' ');
}

Product ProductFormWidget::getProduct() const
{
    Product product;
    auto name = normalizeWords(this->nameForm->text());
    if (name.isEmpty())
        throw std::runtime_error(tr("Name cannot be empty").toStdString());

    product.name = std::move(name);

    auto codeEanText = this->codeEanForm->text();
    if (!codeEanText.isEmpty())
        product.codeEan = CodeEan(codeEanText);

    bool ok = false;
    auto quantity = this->quantityForm->text().toFloat(&ok);
    if (!ok)
        throw std::runtime_error(tr("Invalid quantity value").toStdString());

    product.quantity = quantity;

    product.unitCode = this->unitCodeForm->text().remove(' ').toLower();

    return product;
}

void ProductFormWidget::clear() noexcept
{
    this->nameForm->clear();
    this->codeEanForm->clear();
    this->quantityForm->clear();
    this->unitCodeForm->clear();
}

void ProductFormWidget::focusNameForm() noexcept
{
    this->nameForm->setFocus();
}
