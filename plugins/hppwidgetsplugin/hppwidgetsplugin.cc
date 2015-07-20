#include "hppwidgetsplugin.hh"

#include <QDockWidget>

#include <hpp/gui/mainwindow.h>

#include "pathplayer.h"

HppWidgetsPlugin::HppWidgetsPlugin() :
  hpp_ (new hpp::corbaServer::Client (0,0))
{
  hpp_->connect ();
}

HppWidgetsPlugin::~HppWidgetsPlugin()
{
  if (hpp_)
    delete hpp_;
}

void HppWidgetsPlugin::init()
{
  MainWindow* main = MainWindow::instance ();

  // Path player widget
  QDockWidget* pp_dock = new QDockWidget ("Path player", main);
  PathPlayer* pp = new PathPlayer (this, pp_dock);
  pp_dock->setWidget(pp);
  main->insertDockWidget (pp_dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
}

QString HppWidgetsPlugin::name() const
{
  return QString ("Widgets for hpp-corbaserver");
}

HppWidgetsPlugin::HppClient *HppWidgetsPlugin::client() const
{
  return hpp_;
}

Q_EXPORT_PLUGIN2 (hppwidgetsplugin, HppWidgetsPlugin)
