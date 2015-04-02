#include <QtGui/QApplication>

#include "hpp/gui/mainwindow.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("@PROJECT_NAME@");
  QCoreApplication::setOrganizationDomain("@PROJECT_URL@");
  QCoreApplication::setApplicationName("@PROJECT_NAME@");
  QCoreApplication::setApplicationVersion("@PROJECT_VERSION@");

  QSettings::setPath(QSettings::IniFormat,
                     QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");

  QApplication a(argc, argv);
  a.setStyle(new QCleanlooksStyle);
  MainWindow w;
  w.show();

  return a.exec();
}
