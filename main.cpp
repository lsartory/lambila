#include "MainWindow.h"

#include <QApplication>

/******************************************************************************/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("lila");
    QCoreApplication::setApplicationName("lila");
    MainWindow w;
    w.show();
    return a.exec();
}
