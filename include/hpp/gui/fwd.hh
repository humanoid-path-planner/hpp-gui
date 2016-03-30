#ifndef HPP_GUI_FWD_HH
#define HPP_GUI_FWD_HH

#include <boost/shared_ptr.hpp>
#include <hpp/gui/config-dep.hh>

namespace hpp {
  namespace gui {
    class MainWindow;
    class OSGWidget;
    class BodyTreeWidget;

    class ViewerCorbaServer;

    class WindowsManager;
    typedef boost::shared_ptr <WindowsManager> WindowsManagerPtr_t;

#if HPP_GUI_HAS_PYTHONQT
    class PythonWidget;
#endif
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_FWD_HH
