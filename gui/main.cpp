#include <QtGui/QApplication>
#include "mainwindow.hpp"
#include "newworld.hpp"
#include "widgets.hpp"

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    NewWorldDialog dialog;
    dialog.exec();

    return a.exec();
}

