#ifndef HPP_GUI_FWD_HH
#define HPP_GUI_FWD_HH

#include <boost/shared_ptr.hpp>

namespace hpp {
  namespace gui {
    class MainWindow;
    class OSGWidget;

    class ViewerCorbaServer;

    class WindowsManager;
    typedef boost::shared_ptr <WindowsManager> WindowsManagerPtr_t;
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_FWD_HH
