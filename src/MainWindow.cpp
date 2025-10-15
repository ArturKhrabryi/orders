#include "MainWindow.hpp"
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <QInputDialog>
#include "Database.hpp"
#include "ProductFormWidget.hpp"
#include "ProductsTableView.hpp"
#include "ProcessRunner.hpp"


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
    central(new QWidget(this)),
    view(new ProductsTableView(central)),
    forms(new ProductFormWidget(central)),
    addButton(new QPushButton(tr("&Add the entered product"), central)),
    convertButton(new QPushButton("", central)),
    clearButton(new QPushButton(tr("&Clear the database"), central)),
    barcodeButton(new QPushButton("", central))
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
        QString message = tr("Error: ");
        message.append(ex.what());
        message.append(tr("\nTry again"));

        QMessageBox::warning(this, tr("Failed to add product"), message);
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

    const auto title = tr("Conversion result");
    if (res.ok)
        QMessageBox::information(this, title, tr("Completed successfully: ") + res.output);
    else
        QMessageBox::warning(this, title, tr("Startup error: ") + (res.error.isEmpty() ? res.output : res.error));
}

void MainWindow::onClearAll() noexcept
{
    auto reply = QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to clear the database?"), QMessageBox::Yes | QMessageBox::No); 
    if (reply == QMessageBox::Yes)
    {
        try
        {
            this->db.moveAllToTrash();
            this->refreshModel();
            QMessageBox::information(this, tr("Database cleared"), tr("The database was successfully cleared"));
        }
        catch (const SqlError& ex)
        {
            QMessageBox::warning(this, tr("Database error"), ex.what());
        }
        catch (const std::exception& ex)
        {
            QMessageBox::warning(this, tr("Error"), ex.what());
        }
    }
}

void MainWindow::onGenerateBarcode() noexcept
{
    const auto codeEanText = QInputDialog::getText(this, tr("Enter barcode"), tr("Barcode:"));
    if (codeEanText.simplified().isEmpty())
        return;

    try
    {
        CodeEan codeEan(codeEanText);

        QString filename = QInputDialog::getText(const_cast<MainWindow*>(this), tr("Enter file name"), tr("File name:")).simplified();
        if (filename.isEmpty())
            return;

        const QString generatorName = "barcodeGenerator";
        const auto appDir = QCoreApplication::applicationDirPath();

        const auto res = ProcessRunner::run(
            appDir + "/" + generatorName,
            { "-f", appDir + "/" + filename, "-e", codeEan.getValue() },
            appDir 
        );

        const auto title = tr("Generation result");
        if (res.ok)
            QMessageBox::information(this, title, tr("Completed successfully: ") + res.output);
        else
            QMessageBox::warning(this, title, tr("Startup error: ") + (res.error.isEmpty() ? res.output : res.error));
    }
    catch (const std::exception& ex)
    {
        QMessageBox::warning(this, tr("Something went wrong"), tr("Error: ") + ex.what());
    }
}

void MainWindow::onDeleteSelected() noexcept
{
    auto cur = this->view->currentIndex();
    if (!cur.isValid())
        return;

    auto answer = QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to delete this product?"));
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
        QMessageBox::warning(this, tr("Error while deleting product"), tr("Error: ") + ex.what());

        return;
    }

    this->refreshModel();
}

void MainWindow::buildUi()
{
	auto* mainLayout = new QVBoxLayout(this->central);

	mainLayout->addWidget(this->view);
    mainLayout->addWidget(this->forms);

    auto* cols = new QHBoxLayout;
    mainLayout->addLayout(cols);

    this->convertButton->setIcon(QIcon(":/icons/db_to_sql.svg"));
    this->convertButton->setIconSize(QSize(48, 48));
    this->convertButton->setFixedSize(64, 64);
    cols->addWidget(this->convertButton);

    this->barcodeButton->setIcon(QIcon(":/icons/ean13.svg"));
    this->barcodeButton->setIconSize(QSize(48, 48));
    this->barcodeButton->setFixedSize(64, 64);
    cols->addWidget(this->barcodeButton);

    auto* row = new QVBoxLayout;
    row->addWidget(this->addButton);
    row->addWidget(this->clearButton);
    cols->addLayout(row);

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

