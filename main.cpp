#include "MainWindow.h"
#include "QtScrollAccidentBlocker.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QtScrollAccidentBlocker scAcBl;
    qApp->installEventFilter(&scAcBl);

    MainWindow w;
    w.show();
    return a.exec();
}
