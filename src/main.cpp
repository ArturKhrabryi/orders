#include <QApplication>
#include <QMessageBox>
#include "MainWindow.hpp"


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	try {
        MainWindow window;
        window.show();

        return app.exec();

    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Fatal", e.what());
        return 1;

    } catch (...) {
        QMessageBox::critical(nullptr, "Fatal", "Unknown fatal error");
        return 1;
    }
}
