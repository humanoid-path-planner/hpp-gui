#include <QtGui/QApplication>
#include <QSettings>
#include <QCleanlooksStyle>
#include <QProcessEnvironment>
#include <QSplashScreen>

#include "hpp/gui/safeapplication.hh"
#include "hpp/gui/mainwindow.hh"
#include "hpp/gui/dialog/pluginmanagerdialog.hh"
#include "hpp/gui/settings.hh"

#include <X11/Xlib.h>

using namespace hpp::gui;

void setupEnvironment ()
{
  QCoreApplication::setOrganizationName("@PROJECT_NAME@");
  QCoreApplication::setOrganizationDomain("@PROJECT_URL@");
  QCoreApplication::setApplicationName("@PROJECT_NAME@");
  QCoreApplication::setApplicationVersion("@PROJECT_VERSION@");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (env.contains ("HPP_GUI_SETTINGS_DIR")) {
    QSettings::setPath(QSettings::IniFormat,
        QSettings::SystemScope, env.value("HPP_GUI_SETTINGS_DIR"));
    QSettings::setPath(QSettings::NativeFormat,
        QSettings::SystemScope, env.value("HPP_GUI_SETTINGS_DIR"));
  } else {
    QSettings::setPath(QSettings::IniFormat,
        QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");
    QSettings::setPath(QSettings::NativeFormat,
        QSettings::SystemScope, "@CMAKE_INSTALL_PREFIX@/etc");
  }

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

  QIcon::setThemeName("oxygen");
}

int main(int argc, char *argv[])
{
  XInitThreads();

  SafeApplication a(argc, argv);
  a.setStyle(new QCleanlooksStyle);

  setupEnvironment();

  Settings settings;
  if (!settings.fromArgv (argc, argv)) return 1;
  settings.fromFiles ();

  QPixmap pixmap(":/img/gepetto.png");
  QSplashScreen splash(pixmap);
  splash.show();
  a.processEvents ();

  MainWindow w (&settings);
  settings.setMainWindow (&w);
  settings.initPlugins ();
  w.connect (&a, SIGNAL (log(QString)), SLOT (logError(const QString&)));
  w.show();
  splash.finish(&w);
  return a.exec();
}
