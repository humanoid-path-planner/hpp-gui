#ifndef HPP_GUI_SETTINGS_HH
#define HPP_GUI_SETTINGS_HH

#include <ostream>
#include <string>

struct Settings {
  std::string configurationFile;
  std::string predifinedRobotConf;
  std::string predifinedEnvConf;

  bool verbose;
  bool noPlugin;
  bool autoWriteSettings;

  std::ostream& print (std::ostream& os);
};

#endif // HPP_GUI_SETTINGS_HH
