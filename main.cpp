#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
#include <drumstick.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //This is just the drumstick test! Remove later!! TODO

    MainWindow w;
    w.show();
    return a.exec();
}
