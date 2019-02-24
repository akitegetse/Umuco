#include "caumucomain.h"
#include <QApplication>
#include <QMessageBox>

///
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ///
    /// Check if already running
    ///
    QSharedMemory mem("Umuco");
    if(!mem.create(1)){
        QMessageBox::critical(0,"Umuco.exe detected!",
                              "Application is already running!\n"
                              "Application terminating...","Ok");
        exit(0);
    }

    ///
    /// Saved settings
    ///
    QApplication::setOrganizationName("Université Laval");
    QApplication::setOrganizationDomain("ulaval.ca");
    QApplication::setApplicationName("Umuco");

    ///
    /// Main class
    ///
    CAUmucoMain umuco(&a);
    umuco.openWindow();
    ///
    /// Run
    ///
    return a.exec();
}
