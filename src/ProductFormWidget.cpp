#include "ProductFormWidget.hpp"
#include "CodeEan.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLocale>
#include <QSqlTableModel>
#include <QMessageBox>


ProductFormWidget::ProductFormWidget(QWidget* parent, const QSqlDatabase& db) :
    QWidget(parent),
    nameForm(new QLineEdit(this)),
    codeEanForm(new QLineEdit(this)),
    quantityForm(new QDoubleSpinBox(this)),
    unitCodeForm(new QComboBox(this))
{
    auto* layout = new QVBoxLayout(this);
    auto makeForm = [&](const QString& labelText, QWidget* form) -> void
    {
        auto label = new QLabel(labelText, this);
        label->setBuddy(form);
        layout->addWidget(label);
        layout->addWidget(form);
    };

    makeForm(tr("&Name"), this->nameForm);
    makeForm(tr("&Code ean"), this->codeEanForm);
    makeForm(tr("&Quantity"), this->quantityForm);
    makeForm(tr("&Unit"), this->unitCodeForm);

    this->quantityForm->setDecimals(3);
    this->quantityForm->setMinimum(0.0);
    this->quantityForm->setMaximum(std::numeric_limits<double>::max());
    this->quantityForm->setSingleStep(1.0);

    auto* model = new QSqlTableModel(this, db);
    model->setTable("units");
    model->select();
    this->unitCodeForm->setModel(model);
    this->unitCodeForm->setModelColumn(0);
}

QString ProductFormWidget::normalizeWords(const QString& input) noexcept
{
    auto words = input.simplified().split(' ');
    for (auto& word : words) word = word.left(1).toUpper() + word.mid(1).toLower();

    return words.join(' ');
}

ProductFormData ProductFormWidget::getProductFormData() const
{
    ProductFormData productData;
    productData.name = normalizeWords(this->nameForm->text());

    auto codeEanText = this->codeEanForm->text().simplified();
    if (!codeEanText.isEmpty())
        productData.codeEan = CodeEan(codeEanText);

    productData.quantity = this->quantityForm->value();
    productData.unitCode = this->unitCodeForm->currentText();

    return productData;
}

void ProductFormWidget::clear() noexcept
{
    this->nameForm->clear();
    this->codeEanForm->clear();
    this->quantityForm->setValue(0.0);
    this->unitCodeForm->clear();
}

void ProductFormWidget::focusNameForm() noexcept
{
    this->nameForm->setFocus();
}
