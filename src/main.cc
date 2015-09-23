#include <QtGui/QApplication>
#include <QSettings>
#include <QCleanlooksStyle>
#include <QProcessEnvironment>
#include <QSplashScreen>

#include "hpp/gui/safeapplication.h"
#include "hpp/gui/mainwindow.h"
#include "hpp/gui/dialog/pluginmanagerdialog.h"

#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
  XInitThreads();

  SafeApplication a(argc, argv);
  a.setStyle(new QCleanlooksStyle);
  QIcon::setThemeName("oxygen");

  QPixmap pixmap(":/img/gepetto.png");
  QSplashScreen splash(pixmap);
  splash.show();

  QCoreApplication::setOrganizationName("@PROJECT_NAME@");
  QCoreApplication::setOrganizationDomain("@PROJECT_URL@");
  QCoreApplication::setApplicationName("@PROJECT_NAME@");
  QCoreApplication::setApplicationVersion("@PROJECT_VERSION@");

  QSettings::setPath(QSettings::IniFormat,
                     QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  foreach (QString p, env.value("LD_LIBRARY_PATH").split(':')) {
      PluginManager::addPluginDir (p + "/hpp-gui-plugins");
    }
  // Finally, add all the folders contained in the folder "plugins"
  // in the executable directory. This is convenient for developpers. When
  // running "hpp-gui" from the build directory, the current build plugins are
  // loaded instead of the installed versions.
  // Thus, they do not have to be installed before launching the GUI.
  // TODO: A better way to achieve this would be to use environment variables.
  QDir here (QCoreApplication::applicationDirPath ());
  here.cd ("plugins");
  if (here.exists () && here.isReadable ()) {
    foreach (QString dir, here.entryList (QDir::AllDirs)) {
      PluginManager::addPluginDir (here.absoluteFilePath(dir));
    }
  }

  QStringList args = a.arguments();
  if (args.contains("--help")) {
      std::cout << "--help   " << "\t" << "show this message." << std::endl;
      return 0;
       }

  MainWindow w;
  w.show();
  splash.finish(&w);
  return a.exec();
}
