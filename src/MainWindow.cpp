#include "MainWindow.hpp"
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <QInputDialog>
#include <QFileDialog>
#include <QStringLiteral>
#include <QLocale>
#include "Database.hpp"
#include "DatabaseError.hpp"
#include "ProductFormWidget.hpp"
#include "OrderTableView.hpp"
#include "ProcessRunner.hpp"
#include "MarginCalculator.hpp"
#include "OrderTableModel.hpp"


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
    db(),
    central(new QWidget(this)),
    view(new OrderTableView(central)),
    forms(new ProductFormWidget(central)),
    addButton(new QPushButton(tr("&Add the entered product"), central)),
    convertButton(new QPushButton("", central)),
    clearButton(new QPushButton(tr("&Clear the database"), central)),
    barcodeButton(new QPushButton("", central)),
    marginButton(new QPushButton(tr("&Calculate margin"), central))
{
	this->setCentralWidget(this->central);

    this->buildUi();
    this->createActions();

    this->view->attachModel(this->db.getModel());
    this->refreshModel();
    this->forms->focusNameForm();

	this->setWindowTitle(tr("Orders"));
	this->resize(500, 650);
}

void MainWindow::refreshModel()
{
    auto* orderTableModel = qobject_cast<OrderTableModel*>(view->model());
    if (orderTableModel)
        orderTableModel->select();

    this->view->resizeColumnsToContents();
}

void MainWindow::onAddProduct() noexcept
{
    try
    {
        auto productFormData = this->forms->getProductFormData();
        this->db.addOrderLine(productFormData);
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

        QMessageBox::warning(this, tr("Failed to add order line"), message);
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
            this->db.moveOrderLinesToTrash();
            this->refreshModel();
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

        QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Choose where to save the barcode"),
                QCoreApplication::applicationDirPath(),
                tr("PNG Images (*.png)")
        );

        if (filename.isEmpty())
            return;

        const QString generatorName = QStringLiteral("barcodeGenerator");
        const auto appDir = QCoreApplication::applicationDirPath();

        const auto res = ProcessRunner::run(
            appDir + "/" + generatorName,
            { "-f", filename, "-e", codeEan.getValue() },
            appDir 
        );

        const auto title = tr("Generation result");
        if (res.ok)
            QMessageBox::information(this, title, tr("Barcode generated successfully"));
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
        this->db.moveOrderLineToTrash(id);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while deleting product"), tr("Error: ") + ex.what());

        return;
    }

    this->refreshModel();
}

void MainWindow::onCalculateMargin() noexcept
{
    auto* marginCalculator = new MarginCalculator(this);
    marginCalculator->setAttribute(Qt::WA_DeleteOnClose);
    marginCalculator->setWindowTitle(tr("Margin calculator"));
    marginCalculator->resize(300, 400);
    marginCalculator->exec();
}

void MainWindow::buildUi()
{
	auto* mainLayout = new QVBoxLayout(this->central);

	mainLayout->addWidget(this->view);
    mainLayout->addWidget(this->forms);

    auto* cols = new QHBoxLayout;
    mainLayout->addLayout(cols);

    this->addButton->setStyleSheet("background-color: rgb(0, 100, 0);");
    this->clearButton->setStyleSheet("background-color: rgb(120, 0, 0);");

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
    row->addWidget(this->marginButton);
    cols->addLayout(row);
}

void MainWindow::createActions()
{
	this->connect(this->addButton, &QPushButton::clicked, this, &MainWindow::onAddProduct);
	this->connect(this->convertButton, &QPushButton::clicked, this, &MainWindow::onConvert);
	this->connect(this->clearButton, &QPushButton::clicked, this, &MainWindow::onClearAll);
	this->connect(this->barcodeButton, &QPushButton::clicked, this, &MainWindow::onGenerateBarcode);
    this->connect(this->marginButton, &QPushButton::clicked, this, &MainWindow::onCalculateMargin);

    auto deleteAction = new QAction(this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    this->view->addAction(deleteAction);

    this->connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
}

