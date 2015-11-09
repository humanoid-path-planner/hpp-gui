#include <QtGui/QApplication>
#include <QSettings>
#include <QCleanlooksStyle>
#include <QProcessEnvironment>
#include <QSplashScreen>

#include <boost/program_options.hpp>

#include "hpp/gui/safeapplication.hh"
#include "hpp/gui/mainwindow.hh"
#include "hpp/gui/dialog/pluginmanagerdialog.hh"
#include "hpp/gui/settings.hh"

#include <X11/Xlib.h>

namespace po = boost::program_options;
using namespace hpp::gui;

bool setupProgramOptions (int argc, char *argv[], Settings& s) {
  bool help = false;

  // Declare the supported options.
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("verbose,v", "activate verbose output")

    ("config-file,c",
     po::value<std::string>(&s.configurationFile)->default_value ("settings"),
     "set the configuration file (do not include .conf)")

    ("predefined-robots",
     po::value<std::string>(&s.predifinedRobotConf)->default_value ("robots"),
     "set the predefined robots configuration file (do not include .conf)")

    ("predefined-environments",
     po::value<std::string>(&s.predifinedEnvConf)->default_value ("environments"),
     "set the predefined environments configuration file (do not include .conf)")

    ("no-plugin,P", "do not load any plugin")

    ("auto-write-settings,w", "write the settings in the configuration file")
    ;

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv)
    .options(desc)
    .allow_unregistered()
    .run();
  po::store(parsed, vm);
  po::notify (vm);

  std::vector <std::string> unrecognized =
    po::collect_unrecognized (parsed.options, po::exclude_positional);

  help = (vm.count ("help") > 0);
  s.verbose = (vm.count ("verbose") > 0);
  s.noPlugin = (vm.count ("no-plugin") > 0);
  s.autoWriteSettings = (vm.count ("autoWriteSettings") > 0);

  if (unrecognized.size () > 0) {
    std::cout << "Unrecognized options:\n";
    for (std::size_t i = 0; i < unrecognized.size (); ++i)
      std::cout << unrecognized[i] << "\n";
    std::cout << "\n";
    help = true;
    s.verbose = true;
  }

  if (help) std::cout << desc << std::endl;
  if (s.verbose) s.print (std::cout) << std::endl;

  return !help;
}

int main(int argc, char *argv[])
{
  Settings settings;
  if (!setupProgramOptions (argc, argv, settings)) return 1;

  XInitThreads();

  SafeApplication a(argc, argv);
  a.setStyle(new QCleanlooksStyle);
  QIcon::setThemeName("oxygen");

  QPixmap pixmap(":/img/gepetto.png");
  QSplashScreen splash(pixmap);
  splash.show();
  a.processEvents ();

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

  MainWindow w (settings);
  w.connect (&a, SIGNAL (log(QString)), SLOT (logError(const QString&)));
  w.show();
  splash.finish(&w);
  return a.exec();
}
