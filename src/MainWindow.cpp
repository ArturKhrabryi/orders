#include "MainWindow.hpp"
#include "Product.hpp"
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QShortcut>
#include <QProcess>
#include <QCoreApplication>
#include <exception>
#include <optional>
#include <qmessagebox.h>
#include <stdexcept>


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
    central(new QWidget(this)),
    view(new QTextEdit(central)),
    nameForm(new QLineEdit(central)),
    codeEanForm(new QLineEdit(central)),
    quantityForm(new QLineEdit(central)),
    unitCodeForm(new QLineEdit(central)),
    enterButton(new QPushButton(central)),
    convertButton(new QPushButton(central)),
    deleteButton(new QPushButton(central)),
    clearButton(new QPushButton(central))
{
	this->setCentralWidget(this->central);

	auto* mainLayout = new QVBoxLayout(this->central);
    this->view->setReadOnly(true);
	mainLayout->addWidget(this->view);
    auto* nameLabel = new QLabel("&Nazwa:", this->central);
    nameLabel->setBuddy(this->nameForm);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(this->nameForm);
    auto* codeEanLabel = new QLabel("K&od kreskowy:", this->central);
    codeEanLabel->setBuddy(this->codeEanForm);
    mainLayout->addWidget(codeEanLabel); 
    mainLayout->addWidget(this->codeEanForm);
    auto* quantityLabel = new QLabel("&Ilość", this->central);
    quantityLabel->setBuddy(this->quantityForm);
    mainLayout->addWidget(quantityLabel);
    mainLayout->addWidget(this->quantityForm);
    auto* unitCodeLabel = new QLabel("J&ednostka", this->central);
    unitCodeLabel->setBuddy(this->unitCodeForm);
    mainLayout->addWidget(unitCodeLabel);
    mainLayout->addWidget(this->unitCodeForm);

    auto* removeLayout = new QHBoxLayout;
    removeLayout->addWidget(this->deleteButton);
    removeLayout->addWidget(this->clearButton);
    mainLayout->addLayout(removeLayout);

    auto* logicLayout = new QHBoxLayout;
    logicLayout->addWidget(this->enterButton);
    logicLayout->addWidget(this->convertButton);
    mainLayout->addLayout(logicLayout);

	this->resize(300, 400);
	this->setWindowTitle("Zamówienia");

    this->setPlaceholders();

	this->connect(this->enterButton, &QPushButton::clicked, this, &MainWindow::handleEnterButton);
	this->connect(this->convertButton, &QPushButton::clicked, this, &MainWindow::handleConvertButton);
	this->connect(this->deleteButton, &QPushButton::clicked, this, &MainWindow::handleDeleteButton);
	this->connect(this->clearButton, &QPushButton::clicked, this, &MainWindow::handleClearButton);

    this->updateView();
}

void MainWindow::setPlaceholders() noexcept
{
    this->nameForm->setPlaceholderText("Nazwa");
    this->codeEanForm->setPlaceholderText("Kod kreskowy");
    this->quantityForm->setPlaceholderText("Ilość");
    this->unitCodeForm->setPlaceholderText("Jednostka");

    this->enterButton->setText("&Dodaj wprowadzony towar");
    this->convertButton->setText("&Konwertuj w plik excel");
    this->deleteButton->setText("Usuń &jeden towar z bazy");
    this->clearButton->setText("Usuń &wszystkie towary z bazy");
}

void MainWindow::clearForms() noexcept
{
    this->nameForm->clear();
    this->codeEanForm->clear();
    this->quantityForm->clear();
    this->unitCodeForm->clear();
}

void MainWindow::updateView()
{
    auto products = this->db.fetchProducts(); 
    for (auto& product : products)
        this->view->append(static_cast<QString>(product));
}

Product MainWindow::getProductFromForms() const
{
    Product product;
    product.name = this->getNameFromForm();
    product.codeEan = this->getCodeEanFromForm();
    product.quantity = this->getQuantityFromForm();
    product.unitCode = this->getUnitCodeFromForm();

    return product;
}

QString MainWindow::getNameFromForm() const
{
    auto name = this->normalize(this->nameForm->text());
    if (name.isEmpty())
        throw std::runtime_error("Name cannot be empty");

    return name;
}

QString MainWindow::normalize(const QString& input) noexcept
{
    auto words = input.trimmed().simplified().split(' ');
    for (auto& word : words)
        word = word.left(1).toUpper() + word.mid(1).toLower(); 

    return words.join(' ');
}

std::optional<CodeEan> MainWindow::getCodeEanFromForm() const
{
    auto codeEanText = this->codeEanForm->text();
    if (codeEanText == "-" || codeEanText.isEmpty())
        return std::nullopt;

    return CodeEan(codeEanText);
}

float MainWindow::getQuantityFromForm() const
{
    bool ok = false;
    auto quantity = this->quantityForm->text().toFloat(&ok);
    if (!ok)
        throw std::runtime_error("Invalid quantity value.");

    return quantity;
}

QString MainWindow::getUnitCodeFromForm() const
{
    return this->unitCodeForm->text().remove(' ').toLower();
}

void MainWindow::handleEnterButton() noexcept
{
    try
    {
        auto product = this->getProductFromForms();
        this->db.addProduct(product);
        this->view->append(product);
        this->clearForms();
        this->nameForm->setFocus();
    }
    catch (const std::exception& ex)
    {
        QString message = "Błąd: ";
        message.append(ex.what());
        message.append("\nSpróbuj ponownie");
        QMessageBox::information(this, "Nie udało się dodać towar", message);
    }
}

void MainWindow::handleConvertButton() noexcept
{
    QProcess convertion;
    const QString program = QCoreApplication::applicationDirPath() + "/toOds";
    const QString dbPath = QCoreApplication::applicationDirPath() + "/zamówienie.db";

    convertion.setProgram(program);
    convertion.setArguments({ dbPath });
    convertion.setProcessChannelMode(QProcess::MergedChannels);
    convertion.setWorkingDirectory(QCoreApplication::applicationDirPath());

    convertion.start();
    QString msgBoxTitle = "Wynik konwersji";
    if (!convertion.waitForStarted(3000))
    {
        QMessageBox::warning(this->central, msgBoxTitle, "Nie udało się uruchomić konwertera: " + convertion.errorString());

        return;
    }

    if (!convertion.waitForFinished(3 * 60 * 1000))
    {
        convertion.kill();
        convertion.waitForFinished();
        QMessageBox::warning(this->central,msgBoxTitle, "Przekroczono limit czasu wykonywania.");

        return;
    }

    const int exitCode = convertion.exitCode();
    const auto exitStatus = convertion.exitStatus();
    const auto mergedOutput = QString::fromLocal8Bit(convertion.readAllStandardOutput());
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
        QMessageBox::information(this->central, msgBoxTitle, "Zakończono pomyślnie: " + mergedOutput);

    else
    {
        QString msgBoxBody = "Coś poszło nie tak: " + mergedOutput + "\nKod wyjścia: " + QString::number(exitCode);
        QMessageBox::warning(this->central, msgBoxTitle, msgBoxBody);
    }
}

void MainWindow::handleDeleteButton() noexcept
{

}

void MainWindow::handleClearButton() noexcept
{

}
