#include <QTranslator>
#include <QApplication>
#include <QMessageBox>
#include <iostream>
#include "MainWindow.hpp"


int main(int argc, char* argv[])
{
    QTranslator tr;
    auto res = tr.load(":/i18n/orders_pl");
    if (!res)
        std::cout << "Cannot load translations" << std::endl;

    QApplication app(argc, argv);
    if (res)
        app.installTranslator(&tr);

	try
	{
        MainWindow window;
        window.show();

        return app.exec();
    }
    catch (const std::exception& ex)
    {
        QMessageBox::critical(nullptr, "Fatal", ex.what());

        return 1;
    }
    catch (...)
    {
        QMessageBox::critical(nullptr, "Fatal", "Unknown fatal error");

        return 1;
    }
}

