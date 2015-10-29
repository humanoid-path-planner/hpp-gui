#ifndef HPP_GUI_WINDOWSMANAGER_HH
#define HPP_GUI_WINDOWSMANAGER_HH

#include <hpp/gui/fwd.h>
#include <gepetto/viewer/corba/windows-manager.hh>

class WindowsManager : public graphics::WindowsManager
{
public:
  typedef graphics::WindowsManager Parent_t;

  static WindowsManagerPtr_t create ();

  WindowID createWindow(const char *windowNameCorba);
  WindowID createWindow(const char *windowNameCorba, osg::GraphicsContext *gc);

protected:
  WindowsManager ();
};

#endif // HPP_GUI_WINDOWSMANAGER_HH
