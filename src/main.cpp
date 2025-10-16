#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include "MainWindow.hpp"


static inline void setDarkTheme(QApplication& app)
{
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette dark;
    dark.setColor(QPalette::Window, QColor(53, 53, 53));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(25, 25, 25));
    dark.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
    dark.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
    dark.setColor(QPalette::ToolTipText, Qt::white);
    dark.setColor(QPalette::Text, Qt::white);
    dark.setColor(QPalette::Button, QColor(53, 53, 53));
    dark.setColor(QPalette::ButtonText, Qt::white);
    dark.setColor(QPalette::Link, QColor(42, 130, 218));
    dark.setColor(QPalette::Highlight, QColor(42, 130, 218));
    dark.setColor(QPalette::HighlightedText, Qt::black);
    dark.setColor(QPalette::BrightText, Qt::red);

    app.setPalette(dark);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    setDarkTheme(app);
    
    QLocale plLocale(QLocale::Polish, QLocale::Poland);
    QLocale::setDefault(plLocale);

    QTranslator qtTr;
    if (qtTr.load(plLocale, "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTr);

    QTranslator appTr;
    if (appTr.load(plLocale, "orders", "_", ":/i18n"))
        app.installTranslator(&appTr);

	try
	{
        MainWindow window;
        window.show();

        return app.exec();
    }
    catch (const std::exception& ex)
    {
        QString title = QCoreApplication::translate("ErrorDialog", "Critical Error");
        QString message = QCoreApplication::translate("ErrorDialog", "A critical error occured: ");
        QMessageBox::critical(nullptr, title, message + ex.what());

        return 1;
    }
}

