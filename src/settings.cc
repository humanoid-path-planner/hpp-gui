// Copyright (c) 2015, Joseph Mirabel
// Authors: Joseph Mirabel (joseph.mirabel@laas.fr)
//
// This file is part of hpp-gui.
// hpp-gui is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-gui is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-gui. If not, see <http://www.gnu.org/licenses/>.

#include <hpp/gui/settings.hh>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace hpp {
  namespace gui {
    std::ostream& Settings::print (std::ostream& os) {
      return os
        <<   "Configuration file:     \t" << configurationFile
        << "\nPredefined robots:      \t" << predifinedRobotConf
        << "\nPredefined environments:\t" << predifinedEnvConf
        << "\nVerbose:                \t" << verbose
        << "\nNo plugin:              \t" << noPlugin
        << "\nStart corba server:     \t" << startGepettoCorbaServer
        << "\nRefresh rate:           \t" << refreshRate
           ;
    }

    bool Settings::fromArgv(const int argc, char * const argv[])
    {
      bool help = false;

      // Declare the supported options.
      po::options_description desc("Options");
      desc.add_options()
          ("help,h", "produce help message")
          ("verbose,v", "activate verbose output")

          ("config-file,c",
           po::value<std::string>(&configurationFile)->default_value ("settings"),
           "set the configuration file (do not include .conf)")

          ("predefined-robots",
           po::value<std::string>(&predifinedRobotConf)->default_value ("robots"),
           "set the predefined robots configuration file (do not include .conf)")

          ("predefined-environments",
           po::value<std::string>(&predifinedEnvConf)->default_value ("environments"),
           "set the predefined environments configuration file (do not include .conf)")

          ("no-plugin,P", "do not load any plugin")

          ("auto-write-settings,w", "write the settings in the configuration file")

          ("no-viewer-server", "do not start the Gepetto Viewer server")
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
      verbose = (vm.count ("verbose") > 0);
      noPlugin = (vm.count ("no-plugin") > 0);
      autoWriteSettings = (vm.count ("autoWriteSettings") > 0);
      startGepettoCorbaServer = (vm.count ("no-viewer-server") == 0);
      refreshRate = 40;

      if (unrecognized.size () > 0) {
          std::cout << "Unrecognized options:\n";
          for (std::size_t i = 0; i < unrecognized.size (); ++i)
            std::cout << unrecognized[i] << "\n";
          std::cout << "\n";
          help = true;
          verbose = true;
        }

      if (help) std::cout << desc << std::endl;
      if (verbose) print (std::cout) << std::endl;

      return !help;
    }
  } // namespace gui
} // namespace hpp
