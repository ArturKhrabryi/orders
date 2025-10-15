#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include "MainWindow.hpp"


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
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

