#include "MainWindow.h"

#include <QApplication>

/******************************************************************************/

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("lila");
    QCoreApplication::setApplicationName("lila");
    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
