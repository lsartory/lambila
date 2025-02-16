/* Lambila | main.cpp
 * Copyright (c) 2025 L. Sartory
 * SPDX-License-Identifier: MIT
 */

/******************************************************************************/

#include "MainWindow.h"

#include <QApplication>

/******************************************************************************/

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("lambila");
    QCoreApplication::setApplicationName("lambila");
    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
