#include "hppwidgetsplugin.hh"

#include <QDockWidget>

#include <hpp/gui/mainwindow.h>
#include <hpp/gui/windows-manager.h>

#include <omniORB4/CORBA.h>

#include "pathplayer.h"
#include "solverwidget.h"
#include "jointtreewidget.h"
#include "configurationlistwidget.h"
#include "joint-tree-item.h"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

using CORBA::ULong;

HppWidgetsPlugin::HppWidgetsPlugin() :
  pathPlayer_ (NULL),
  solverWidget_ (NULL),
  jointTreeWidget_ (NULL),
  configListWidget_ (NULL),
  hpp_ (new hpp::corbaServer::Client (0,0))
{
  hpp_->connect ();
}

HppWidgetsPlugin::~HppWidgetsPlugin()
{
  MainWindow* main = MainWindow::instance ();
  foreach (QDockWidget* dock, dockWidgets_) {
      main->removeDockWidget(dock);
      delete dock;
    }
  if (hpp_)
    delete hpp_;
}

void HppWidgetsPlugin::init()
{
  MainWindow* main = MainWindow::instance ();
  QDockWidget* dock;

  // Path player widget
  dock = new QDockWidget ("Path player", main);
  pathPlayer_ = new PathPlayer (this, dock);
  dock->setWidget(pathPlayer_);
  main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
  dockWidgets_.append(dock);

  // Solver widget
  dock = new QDockWidget ("Problem solver", main);
  solverWidget_ = new SolverWidget (this, dock);
  dock->setWidget(solverWidget_);
  main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
  dockWidgets_.append(dock);

  // Joint tree widget
  dock = new QDockWidget ("Joint Tree", main);
  jointTreeWidget_ = new JointTreeWidget (this, dock);
  dock->setWidget(jointTreeWidget_);
  jointTreeWidget_->dockWidget (dock);
  main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
  dockWidgets_.append(dock);

  // Configuration list widget
  dock = new QDockWidget ("Configuration List", main);
  configListWidget_ = new ConfigurationListWidget (this, dock);
  dock->setWidget(configListWidget_);
  main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
  dockWidgets_.append(dock);

  // Connect widgets
  connect (solverWidget_, SIGNAL (problemSolved ()), pathPlayer_, SLOT (update()));

  connect (jointTreeWidget_->refreshButton (), SIGNAL (clicked ()),
           main, SLOT (reload()));
  connect (main, SIGNAL (configurationValidation ()),
           this, SLOT (configurationValidation ()));
  connect (this, SIGNAL (configurationValidationStatus (bool)),
           main, SLOT (configurationValidationStatusChanged (bool)));
  connect (main, SIGNAL (applyCurrentConfiguration()),
           this, SLOT (applyCurrentConfiguration()));
  connect (main, SIGNAL (selectJointFromBodyName (std::string)),
           this, SLOT (selectJointFromBodyName (std::string)));
  connect (this, SIGNAL (logJobFailed(int,QString)),
           main, SLOT (logJobFailed(int, QString)));
}

QString HppWidgetsPlugin::name() const
{
  return QString ("Widgets for hpp-corbaserver");
}

void HppWidgetsPlugin::loadRobotModel(DialogLoadRobot::RobotDefinition rd)
{
  client()->robot()->loadRobotModel(
        QSTRING_TO_CONSTCHARARRAY(rd.name_),
        QSTRING_TO_CONSTCHARARRAY(rd.rootJointType_),
        QSTRING_TO_CONSTCHARARRAY(rd.package_),
        QSTRING_TO_CONSTCHARARRAY(rd.modelName_),
        QSTRING_TO_CONSTCHARARRAY(rd.urdfSuf_),
        QSTRING_TO_CONSTCHARARRAY(rd.srdfSuf_));
  std::string bjn;
  if (rd.rootJointType_.compare("freeflyer") == 0)   bjn = "base_joint_xyz";
  else if (rd.rootJointType_.compare("planar") == 0) bjn = "base_joint_xy";
  else if (rd.rootJointType_.compare("anchor") == 0) bjn = "base_joint";
  updateRobotJoints (rd.robotName_);
  jointTreeWidget_->addJointToTree(bjn, 0);
  applyCurrentConfiguration();
  MainWindow::instance()->logJobDone (0, "Robot " + rd.name_ + " loaded");
}

void HppWidgetsPlugin::loadEnvironmentModel(DialogLoadEnvironment::EnvironmentDefinition ed)
{
  QString prefix = ed.envName_ + "/";
  client()->obstacle()->loadObstacleModel(
        QSTRING_TO_CONSTCHARARRAY(ed.package_),
        QSTRING_TO_CONSTCHARARRAY(ed.urdfFilename_),
        QSTRING_TO_CONSTCHARARRAY(prefix));
  computeObjectPosition ();
  MainWindow::instance()->logJobDone (0, "Environment " + ed.name_ + " loaded");
}

std::string HppWidgetsPlugin::getBodyFromJoint(const std::string &jointName) const
{
  JointMap::const_iterator itj = jointMap_.find(jointName);
  if (itj == jointMap_.constEnd()) return std::string();
  return itj->bodyName;
}

bool HppWidgetsPlugin::corbaException(int jobId, const CORBA::Exception &excep) const
{
  try {
    const hpp::Error& error = dynamic_cast <const hpp::Error&> (excep);
    emit logJobFailed(jobId, QString (error.msg));
    return true;
  } catch (const std::exception& exp) {
    qDebug () << exp.what();
  }
  return false;
}

void HppWidgetsPlugin::applyCurrentConfiguration()
{
  MainWindow * main = MainWindow::instance ();
  main->statusBar()->showMessage("Applying current configuration...");
  if (jointMap_.isEmpty()) {
      main->logError("The current configuration cannot be applied. This is probably because you are using external commands (python interface) and you did not refresh this GUI."
                     " Use the refresh button of the \"Joint Tree\" widget.");
    }
  float T[7];
  for (JointMap::iterator ite = jointMap_.begin ();
       ite != jointMap_.end (); ite++) {
      hpp::Transform__var t = client()->robot()->getLinkPosition(ite->name.c_str());
      for (size_t i = 0; i < 7; ++i) T[i] = (float)t[(ULong)i];
      if (ite->updateViewer)
          ite->updateViewer = main->osg()->applyConfiguration(ite->bodyName.c_str(), T);
      if (!ite->item) continue;
      hpp::floatSeq_var c = client()->robot ()->getJointConfig (ite->name.c_str());
      ite->item->updateConfig (c.in());
    }
  main->osg()->refresh();
  main->statusBar()->clearMessage();
}

void HppWidgetsPlugin::configurationValidation()
{
  hpp::floatSeq_var q = client()->robot()->getCurrentConfig ();
  bool bb = false;
  CORBA::Boolean_out b = bb;
  CORBA::String_var report;
  try {
    client()->robot()->isConfigValid (q.in(), b, report);
  } catch (const hpp::Error& e) {
    MainWindow::instance()->logError(QString (e.msg));
    return;
  }
  emit configurationValidationStatus (b);
}

void HppWidgetsPlugin::selectJointFromBodyName(const std::string &bodyName)
{
  foreach (const JointElement& je, jointMap_) {
      if (bodyName.compare(je.bodyName) == 0) {
          jointTreeWidget_->selectJoint (je.name);
          return;
        }
    }
  qDebug () << "Joint for body" << QString::fromStdString(bodyName) << "not found.";
}

QList<QAction *> HppWidgetsPlugin::getJointActions(const std::string &jointName)
{
  QList <QAction*> l;
  JointAction* a;
  a= new JointAction (tr("Set bounds..."), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), jointTreeWidget_, SLOT (openJointBoundDialog(std::string)));
  l.append(a);
  a = new JointAction (tr("Display roadmap"), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), solverWidget_, SLOT (displayRoadmap(std::string)));
  l.append(a);
  return l;
}

HppWidgetsPlugin::HppClient *HppWidgetsPlugin::client() const
{
  return hpp_;
}

HppWidgetsPlugin::JointMap &HppWidgetsPlugin::jointMap()
{
  return jointMap_;
}

void HppWidgetsPlugin::updateRobotJoints(const QString robotName)
{
  hpp::Names_t_var joints = client()->robot()->getAllJointNames ();
  for (size_t i = 0; i < joints->length (); ++i) {
    const char* jname = joints[(ULong) i];
      const char* lname = client()->robot()->getLinkName (jname);
      std::string linkName = robotName.toStdString() + "/" + std::string (lname);
      jointMap_[jname] = JointElement(jname, linkName, 0, true);
      delete[] lname;
    }
}

void HppWidgetsPlugin::computeObjectPosition()
{
  MainWindow* main = MainWindow::instance ();
  hpp::Names_t_var obs = client()->obstacle()->getObstacleNames (true, false);
  hpp::Transform__out cfg = hpp::Transform__alloc () ;
  float d[7];
  for (size_t i = 0; i < obs->length(); ++i) {
      client()->obstacle()->getObstaclePosition (obs[(ULong) i], cfg);
      for (size_t j = 0; j < 7; j++) d[j] = (float)cfg[j];
      const char* name = obs[(ULong) i];
      main->osg ()->applyConfiguration(name, d);
    }
  main->osg()->refresh();
  delete cfg;
}

Q_EXPORT_PLUGIN2 (hppwidgetsplugin, HppWidgetsPlugin)
