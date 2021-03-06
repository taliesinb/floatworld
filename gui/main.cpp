#include <QtGui/QApplication>
#include "mainwindow.hpp"
#include "newworld.hpp"
#include "widgets.hpp"

#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    NewWorldDialog dialog;
    dialog.CreateDefaultObjects();
    dialog.show();

    return a.exec();
}

