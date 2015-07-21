#include "hppwidgetsplugin.hh"

#include <QDockWidget>

#include <hpp/gui/mainwindow.h>

#include "pathplayer.h"
#include "solverwidget.h"
#include "configurationlistwidget.h"

HppWidgetsPlugin::HppWidgetsPlugin() :
  pathPlayer_ (NULL),
  solverWidget_ (NULL),
  configListWidget_ (NULL),
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
  pathPlayer_ = new PathPlayer (this, pp_dock);
  pp_dock->setWidget(pathPlayer_);
  main->insertDockWidget (pp_dock, Qt::BottomDockWidgetArea, Qt::Horizontal);

  // Solver widget
  QDockWidget* sw_dock = new QDockWidget ("Problem solver", main);
  solverWidget_ = new SolverWidget (this, sw_dock);
  sw_dock->setWidget(solverWidget_);
  main->insertDockWidget (sw_dock, Qt::BottomDockWidgetArea, Qt::Horizontal);

  // Configuration list widget
  QDockWidget* cl_dock = new QDockWidget ("Configuration List", main);
  configListWidget_ = new ConfigurationListWidget (this, cl_dock);
  cl_dock->setWidget(configListWidget_);
  main->insertDockWidget (cl_dock, Qt::RightDockWidgetArea, Qt::Vertical);

  // Connect widgets
  connect (solverWidget_, SIGNAL (problemSolved ()), pathPlayer_, SLOT (update()));
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
