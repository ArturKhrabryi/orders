#include "MainWindow.hpp"
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <QInputDialog>
#include <qnamespace.h>
#include "Database.hpp"
#include "ProductFormWidget.hpp"
#include "ProductsTableView.hpp"
#include "ProcessRunner.hpp"


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
    central(new QWidget(this)),
    view(new ProductsTableView(central)),
    forms(new ProductFormWidget(central)),
    addButton(new QPushButton("&Dodaj wprowadzony towar", central)),
    convertButton(new QPushButton("K&onwertuj w plik excel", central)),
    clearButton(new QPushButton("&Usuń wszystkie towary z bazy", central)),
    barcodeButton(new QPushButton("&Generuj kod kreskowy", central))
{
	this->setCentralWidget(this->central);

    this->buildUi();
    this->createActions();

    this->view->attachModel(this->db.getModel());
    this->refreshModel();
    this->forms->focusNameForm();

	this->setWindowTitle(tr("Orders"));
	this->resize(300, 400);
}

void MainWindow::refreshModel()
{
    auto* databaseModel = qobject_cast<QSqlTableModel*>(view->model());
    if (databaseModel)
        databaseModel->select();

    this->view->resizeColumnsToContents();
}

void MainWindow::onAddProduct() noexcept
{
    try
    {
        auto product = this->forms->getProduct();
        this->db.add(product);
        this->refreshModel();
        this->forms->clear();
        this->forms->focusNameForm();

		this->view->scrollToBottom();
    }
    catch (const std::exception& ex)
    {
        QString message = "Błąd: ";
        message.append(ex.what());
        message.append("\nSpróbuj ponownie");

        QMessageBox::warning(this, "Nie udało się dodać towar", message);
    }
}

void MainWindow::onConvert() noexcept
{
    const QString converterName = "toOds";
    const QString dbName = "order.db";
    const auto appDir = QCoreApplication::applicationDirPath();

    const auto res = ProcessRunner::run(
    appDir + "/" + converterName,
    { appDir + "/" + dbName },
    appDir
    );

    const auto title = QStringLiteral("Wynik konwersji");
    if (res.ok)
        QMessageBox::information(this, title, "Zakończono pomyślnie: " + res.output);
    else
        QMessageBox::warning(this, title, "Błąd uruchomienia: " + (res.error.isEmpty() ? res.output : res.error));
}

void MainWindow::onClearAll() noexcept
{
    auto reply = QMessageBox::question(this, "Potwierdzenie", "Czy na pewno usunąć bazę?", QMessageBox::Yes | QMessageBox::No); 
    if (reply == QMessageBox::Yes)
    {
        try
        {
            this->db.moveAllToTrash();
            this->refreshModel();
            QMessageBox::information(this, "Usunięto bazę", "Towary zostały pomyślnie usunięte");
        }
        catch (const SqlError& ex)
        {
            QMessageBox::warning(this, "Błąd w bazie towarów", ex.what());
        }
        catch (const std::exception& ex)
        {
            QMessageBox::warning(this, "Błąd", ex.what());
        }
        catch (...)
        {
            QMessageBox::critical(this, "Niewiadomy błąd", "Niewiadomy błąd");
        }
    }
}

void MainWindow::onGenerateBarcode() noexcept
{
    const auto codeEanText = QInputDialog::getText(this, "Wprowadź kod kreskowy", "Kod kreskowy:");
    if (codeEanText.simplified().isEmpty())
        return;

    try
    {
        CodeEan codeEan(codeEanText);

        QString filename = QInputDialog::getText(const_cast<MainWindow*>(this), "Wprowadź nazwę pliku", "Nazwa pliku:").simplified();
        if (filename.isEmpty())
            return;

        const QString generatorName = "barcodeGenerator";
        const auto appDir = QCoreApplication::applicationDirPath();

        const auto res = ProcessRunner::run(
            appDir + "/" + generatorName,
            { "-f", appDir + "/" + filename, "-e", codeEan.getValue() },
            appDir 
        );

        const auto title = QStringLiteral("Wynik generacji");
        if (res.ok)
            QMessageBox::information(this, title, "Zakończono pomyślnie: " + res.output);
        else
            QMessageBox::warning(this, title, "Błąd uruchomienia: " + (res.error.isEmpty() ? res.output : res.error));
    }
    catch (const std::exception& ex)
    {
        QMessageBox::warning(this, "Coś poszło nie tak", QStringLiteral("Błąd") + ex.what());
    }
}

void MainWindow::onDeleteSelected() noexcept
{
    auto cur = this->view->currentIndex();
    if (!cur.isValid())
        return;

    auto answer = QMessageBox::question(this, "Potwierdzenie", "Czy na pewno chcesz usunąć ten towar?");
    if (answer == QMessageBox::No)
        return;

    int row = cur.row();
    const int idColumn = 0;
    bool ok = false;
    const int id = this->view->model()->index(row, idColumn).data(Qt::EditRole).toInt(&ok);
    if (!ok)
        return;

    try
    {
        this->db.moveToTrash(id);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::warning(this, "Błąd przy usuwaniu elementu", QStringLiteral("Błąd: ") + ex.what());

        return;
    }

    this->refreshModel();
}

void MainWindow::buildUi()
{
	auto* mainLayout = new QVBoxLayout(this->central);

	mainLayout->addWidget(this->view);
    mainLayout->addWidget(this->forms);

    auto* row1 = new QHBoxLayout;
    row1->addWidget(this->addButton);
    row1->addWidget(this->clearButton);
    mainLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    this->convertButton->setText("");
    this->convertButton->setIcon(QIcon(":/icons/database.svg"));
    this->convertButton->setIconSize(QSize(24, 24));
    row2->addWidget(this->convertButton);
    row2->addWidget(this->barcodeButton);
    mainLayout->addLayout(row2);
}

void MainWindow::createActions()
{
	this->connect(this->addButton, &QPushButton::clicked, this, &MainWindow::onAddProduct);
	this->connect(this->convertButton, &QPushButton::clicked, this, &MainWindow::onConvert);
	this->connect(this->clearButton, &QPushButton::clicked, this, &MainWindow::onClearAll);
	this->connect(this->barcodeButton, &QPushButton::clicked, this, &MainWindow::onGenerateBarcode);

    auto deleteAction = new QAction(this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    this->view->addAction(deleteAction);

    this->connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
}

