#include <QtGui/QApplication>

#include "hpp/gui/mainwindow.h"
#include "hpp/gui/dialog/pluginmanagerdialog.h"

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
  QIcon::setThemeName("oxygen");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  foreach (QString p, env.value("LD_LIBRARY_PATH").split(':')) {
      PluginManager::addPluginDir (p + "/hpp-gui-plugins");
    }

  QStringList args = a.arguments();
  if (args.contains("--help")) {
      std::cout << "--help   " << "\t" << "show this message." << std::endl;
      return 0;
       }

  MainWindow w;
  w.show();

  return a.exec();
}
