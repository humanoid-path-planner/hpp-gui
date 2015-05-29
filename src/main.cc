#include <QtGui/QApplication>

#include "hpp/gui/mainwindow.h"

#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
  XInitThreads();

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

  QStringList args = a.arguments();
  if (args.contains("--help")) {
      std::cout << "--help   " << "\t" << "show this message." << std::endl;
      std::cout << "--hpp-off" << "\t" << "turns off the HPP server." << std::endl;
      return 0;
       }
  bool hppoff = args.contains("--hpp-off");
  if (hppoff)
    std::cout << "The HPP server won't start automatically."
              << "It should have been started before." << std::endl;

  MainWindow w (0, !hppoff);
  w.show();

  return a.exec();
}
