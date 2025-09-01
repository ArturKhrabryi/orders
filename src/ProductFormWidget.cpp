#include "ProductFormWidget.hpp"
#include "CodeEan.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <qboxlayout.h>


ProductFormWidget::ProductFormWidget(QWidget* parent) :
    QWidget(parent),
    nameForm(new QLineEdit(this)),
    codeEanForm(new QLineEdit(this)),
    quantityForm(new QLineEdit(this)),
    unitCodeForm(new QLineEdit(this))
{
    auto* layout = new QVBoxLayout(this);

    auto makeForm = [&](const char* labelText, const char* placeholder, QLineEdit* form) -> void
    {
        auto label = new QLabel(labelText, this);
        label->setBuddy(form);
        layout->addWidget(label);
        layout->addWidget(form);
        form->setPlaceholderText(placeholder);
    };

    makeForm("&Nazwa", "Nazwa", this->nameForm);
    makeForm("&Kod kreskowy", "Kod kreskowy", this->codeEanForm);
    makeForm("&Ilość", "Ilość", this->quantityForm);
    makeForm("&Jednostka", "Jednostka", this->unitCodeForm);
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
        throw std::runtime_error("Name cannot be empty");

    product.name = std::move(name);

    auto codeEanText = this->codeEanForm->text();
    if (!codeEanText.isEmpty())
        product.codeEan = CodeEan(codeEanText);

    bool ok = false;
    auto quantity = this->quantityForm->text().toFloat(&ok);
    if (!ok)
        throw std::runtime_error("Invalid quantity value.");

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
