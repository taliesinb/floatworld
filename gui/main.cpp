#include <QtGui/QApplication>
#include "mainwindow.hpp"
#include "widgets.hpp"

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow win;
    win.show();
    srand(100);

    return a.exec();
}
