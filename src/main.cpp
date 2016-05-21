#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("UltraEdit");
    a.setOrganizationName("MH-Development");

    MainWindow w;
    w.show();
    return a.exec();
}
