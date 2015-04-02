#include "hpp/gui/windows-manager.h"

#include <gepetto/viewer/window-manager.h>

#include "hpp/gui/osgwidget.h"
#include "hpp/gui/mainwindow.h"

WindowsManagerPtr_t WindowsManager::create()
{
  return WindowsManagerPtr_t (new WindowsManager());
}

WindowsManager::WindowID WindowsManager::createWindow(const char *windowNameCorba)
{
  return MainWindow::instance()->delayedCreateView(QString (windowNameCorba))->windowID();
}

WindowsManager::WindowID WindowsManager::createWindow(const char *windowNameCorba, osg::GraphicsContext *gc)
{
  std::string wn (windowNameCorba);
  graphics::WindowManagerPtr_t newWindow = graphics::WindowManager::create (gc);
  WindowID windowId = addWindow (wn, newWindow);
  return windowId;
}

WindowsManager::WindowsManager()
  : Parent_t ()
{
}
