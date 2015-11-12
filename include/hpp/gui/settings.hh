#ifndef HPP_GUI_SETTINGS_HH
#define HPP_GUI_SETTINGS_HH

#include <ostream>
#include <string>

namespace hpp {
  namespace gui {
    struct Settings {
      std::string configurationFile;
      std::string predifinedRobotConf;
      std::string predifinedEnvConf;

      bool verbose;
      bool noPlugin;
      bool autoWriteSettings;
      bool startGepettoCorbaServer;

      int refreshRate;

      std::ostream& print (std::ostream& os);

      bool fromArgv (const int argc, char* const argv[]);
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_SETTINGS_HH
