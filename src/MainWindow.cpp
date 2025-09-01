#include "MainWindow.hpp"
#include "CodeEan.hpp"
#include "Product.hpp"
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QShortcut>
#include <QTableView>
#include <QProcess>
#include <QCoreApplication>
#include <QInputDialog>
#include <exception>
#include <optional>
#include <QHeaderView>
#include <QTableView>
#include <qabstractitemmodel.h>
#include <qkeysequence.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <stdexcept>
#include "Database.hpp"
#include <QHeaderView>


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
    central(new QWidget(this)),
    view(new QTableView(central)),
    nameForm(new QLineEdit(central)),
    codeEanForm(new QLineEdit(central)),
    quantityForm(new QLineEdit(central)),
    unitCodeForm(new QLineEdit(central)),
    enterButton(new QPushButton(central)),
    convertButton(new QPushButton(central)),
    clearButton(new QPushButton(central)),
    barcodeGenerationButton(new QPushButton(central))
{
	this->setCentralWidget(this->central);

	auto* mainLayout = new QVBoxLayout(this->central);
    this->view->setModel(this->db.getModel());
    this->view->setColumnHidden(0, true);
    this->view->setSelectionBehavior(QAbstractItemView::SelectItems);
    this->view->setSelectionMode(QAbstractItemView::SingleSelection);
    this->view->setEditTriggers(QAbstractItemView::DoubleClicked);
    this->view->verticalHeader()->setVisible(false);
    auto* header = this->view->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    this->view->setAlternatingRowColors(true);
    this->view->setItemDelegateForColumn(2, new DatabaseProductsModel::EanDelegate(this->view));

	mainLayout->addWidget(this->view);
    auto* nameLabel = new QLabel("&Nazwa:", this->central);
    nameLabel->setBuddy(this->nameForm);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(this->nameForm);
    auto* codeEanLabel = new QLabel("&Kod kreskowy:", this->central);
    codeEanLabel->setBuddy(this->codeEanForm);
    mainLayout->addWidget(codeEanLabel); 
    mainLayout->addWidget(this->codeEanForm);
    auto* quantityLabel = new QLabel("&Ilość", this->central);
    quantityLabel->setBuddy(this->quantityForm);
    mainLayout->addWidget(quantityLabel);
    mainLayout->addWidget(this->quantityForm);
    auto* unitCodeLabel = new QLabel("&Jednostka", this->central);
    unitCodeLabel->setBuddy(this->unitCodeForm);
    mainLayout->addWidget(unitCodeLabel);
    mainLayout->addWidget(this->unitCodeForm);

    auto* firstLayout = new QHBoxLayout;
    firstLayout->addWidget(this->enterButton);
    firstLayout->addWidget(this->clearButton);
    mainLayout->addLayout(firstLayout);

    auto* secondLayout = new QHBoxLayout;
    secondLayout->addWidget(this->convertButton);
    secondLayout->addWidget(this->barcodeGenerationButton);
    mainLayout->addLayout(secondLayout);

	this->resize(300, 400);
	this->setWindowTitle("Zamówienia");

    this->setPlaceholders();
    this->nameForm->setFocus();

	this->connect(this->enterButton, &QPushButton::clicked, this, &MainWindow::handleEnterButton);
	this->connect(this->convertButton, &QPushButton::clicked, this, &MainWindow::handleConvertButton);
	this->connect(this->clearButton, &QPushButton::clicked, this, &MainWindow::handleClearButton);
	this->connect(this->barcodeGenerationButton, &QPushButton::clicked, this, &MainWindow::handleBarcodeGenerationButton);

    this->createDeleteAction();

    this->updateView();
}

void MainWindow::setPlaceholders() noexcept
{
    this->nameForm->setPlaceholderText("Nazwa");
    this->codeEanForm->setPlaceholderText("Kod kreskowy");
    this->quantityForm->setPlaceholderText("Ilość");
    this->unitCodeForm->setPlaceholderText("Jednostka");

    this->enterButton->setText("&Dodaj wprowadzony towar");
    this->convertButton->setText("K&onwertuj w plik excel");
    this->clearButton->setText("&Usuń wszystkie towary z bazy");
    this->barcodeGenerationButton->setText("&Generuj kod kreskowy");
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
    auto* databaseModel = qobject_cast<QSqlTableModel*>(view->model());
    if (databaseModel)
        databaseModel->select();

    this->view->resizeColumnsToContents();
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
        this->db.add(product);
        this->updateView();
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

void MainWindow::handleConvertButton() const noexcept
{
    QProcess convertion;
    const QString program = QCoreApplication::applicationDirPath() + "/toOds";
    const QString dbPath = QCoreApplication::applicationDirPath() + "/order.db";

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

void MainWindow::handleClearButton() noexcept
{
    auto reply = QMessageBox::question(this, "Potwierdzenie", "Czy na pewno usunąć bazę?", QMessageBox::Yes | QMessageBox::No); 
    if (reply == QMessageBox::Yes)
    {
        try
        {
            this->db.moveAllToTrash();
            this->updateView();
            QMessageBox::information(this, "Usunięto bazę", "Towary zostały pomyślnie usunięte");
        }
        catch (const SqlError& ex)
        {
            QMessageBox::information(this, "Błąd w bazie towarów", ex.what());
        }
        catch (const std::exception& ex)
        {
            QMessageBox::information(this, "Błąd", ex.what());
        }
        catch (...)
        {
            QMessageBox::information(this, "Niewiadomy błąd", "Niewiadomy błąd");
        }
    }
}

void MainWindow::handleBarcodeGenerationButton() noexcept
{
    auto codeEanText = QInputDialog::getText(this, "Wprowadź kod kreskowy", "Kod kreskowy:");
    if (codeEanText.isEmpty())
        return;

    try
    {
        CodeEan codeEan(codeEanText);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::warning(this, "Zły kod kreskowy", QString("Błąd: ") + ex.what());

        return;
    }

    QString filename = QInputDialog::getText(const_cast<MainWindow*>(this), "Wprowadź nazwę pliku", "Nazwa pliku:");
    if (filename.isEmpty())
        return;

    QProcess generation;
    const QString program = QCoreApplication::applicationDirPath() + "/barcodeGenerator";

    generation.setProgram(program);
    QStringList args;
    args << "-f" << filename << "-e" << codeEanText;
    generation.setArguments(args);
    generation.setProcessChannelMode(QProcess::MergedChannels);
    generation.setWorkingDirectory(QCoreApplication::applicationDirPath());

    generation.start();
    QString msgBoxTitle = "Wynik generacji";
    if (!generation.waitForStarted(3000))
    {
        QMessageBox::warning(this->central, msgBoxTitle, "Nie udało się uruchomić generatora: " + generation.errorString());

        return;
    }

    if (!generation.waitForFinished(3 * 60 * 1000))
    {
        generation.kill();
        generation.waitForFinished();
        QMessageBox::warning(this->central,msgBoxTitle, "Przekroczono limit czasu wykonywania.");

        return;
    }

    const int exitCode = generation.exitCode();
    const auto exitStatus = generation.exitStatus();
    const auto mergedOutput = QString::fromLocal8Bit(generation.readAllStandardOutput());
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
        QMessageBox::information(this->central, msgBoxTitle, "Zakończono pomyślnie: " + mergedOutput);

    else
    {
        QString msgBoxBody = "Coś poszło nie tak: " + mergedOutput + "\nKod wyjścia: " + QString::number(exitCode);
        QMessageBox::warning(this->central, msgBoxTitle, msgBoxBody);
    }
}

void MainWindow::handleDeleteItem() noexcept
{
    auto answer = QMessageBox::question(this, "Potwierdzenie", "Czy na pewno chcesz usunąć ten towar?");
    if (answer == QMessageBox::No)
        return;

    auto cur = this->view->currentIndex();
    if (!cur.isValid())
        return;

    const int idColumn = 0;
    int row = cur.row();

    auto idIdx = this->view->model()->index(row, idColumn);
    bool ok = false;
    int id = this->view->model()->data(idIdx, Qt::EditRole).toInt(&ok);
    if (!ok)
        return;

    try
    {
        this->db.moveToTrash(id);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::warning(this, "Błąd przy usuwaniu elementu", QString("Błąd: ") + ex.what());

        return;
    }

    this->updateView();
}

void MainWindow::createDeleteAction() noexcept
{
    auto deleteAction = new QAction(this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    this->view->addAction(deleteAction);

    this->connect(deleteAction, &QAction::triggered, this, &MainWindow::handleDeleteItem);
}
