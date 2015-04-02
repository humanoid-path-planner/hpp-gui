#ifndef FWD_H
#define FWD_H

#include <boost/shared_ptr.hpp>

class MainWindow;

class JointTreeItem;

class SolverWidget;
class PathPlayer;
class ConfigurationListWidget;

class HppCorbaServer;
class ViewerCorbaServer;

class WindowsManager;
typedef boost::shared_ptr <WindowsManager> WindowsManagerPtr_t;

#endif // FWD_H
