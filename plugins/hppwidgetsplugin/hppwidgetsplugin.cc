#include "hppwidgetsplugin/hppwidgetsplugin.hh"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <QDockWidget>

#include <hpp/gui/mainwindow.hh>
#include <hpp/gui/windows-manager.hh>

#include <omniORB4/CORBA.h>

#include "hppwidgetsplugin/pathplayer.hh"
#include "hppwidgetsplugin/solverwidget.hh"
#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppwidgetsplugin/configurationlistwidget.hh"
#include "hppwidgetsplugin/joint-tree-item.hh"

#include "hppwidgetsplugin/roadmap.hh"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

using CORBA::ULong;

HppWidgetsPlugin::HppWidgetsPlugin() :
  pathPlayer_ (NULL),
  solverWidget_ (NULL),
  jointTreeWidget_ (NULL),
  configListWidget_ (NULL),
  hpp_ (NULL)
{
}

HppWidgetsPlugin::~HppWidgetsPlugin()
{
  MainWindow* main = MainWindow::instance ();
  foreach (QDockWidget* dock, dockWidgets_) {
      main->removeDockWidget(dock);
      delete dock;
    }
  closeConnection ();
}

void HppWidgetsPlugin::init()
{
  openConnection();

  MainWindow* main = MainWindow::instance ();
  QDockWidget* dock;

  // Configuration list widget
  dock = new QDockWidget ("&Configuration List", main);
  configListWidget_ = new ConfigurationListWidget (this, dock);
  dock->setWidget(configListWidget_);
  main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
  dock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
  dockWidgets_.append(dock);

  // Solver widget
  dock = new QDockWidget ("Problem &solver", main);
  solverWidget_ = new SolverWidget (this, dock);
  dock->setWidget(solverWidget_);
  main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
  main->tabifyDockWidget(dockWidgets_.first(), dock);
  dock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_S);
  dockWidgets_.append(dock);

  // Path player widget
  dock = new QDockWidget ("&Path player", main);
  pathPlayer_ = new PathPlayer (this, dock);
  dock->setWidget(pathPlayer_);
  main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
  main->tabifyDockWidget(dockWidgets_.first(), dock);
  dock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_P);
  dockWidgets_.append(dock);

  // Joint tree widget
  dock = new QDockWidget ("&Joint Tree", main);
  jointTreeWidget_ = new JointTreeWidget (this, dock);
  dock->setWidget(jointTreeWidget_);
  jointTreeWidget_->dockWidget (dock);
  main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
  dock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_J);
  dockWidgets_.append(dock);

  // Connect widgets
  connect (solverWidget_, SIGNAL (problemSolved ()), pathPlayer_, SLOT (update()));
  connect (main, SIGNAL (refresh()), jointTreeWidget_, SLOT (reload ()));
  connect (main, SIGNAL (refresh()), pathPlayer_, SLOT (update()));

  connect (main, SIGNAL (configurationValidation ()),
           this, SLOT (configurationValidation ()));
  connect (this, SIGNAL (configurationValidationStatus (bool)),
           main, SLOT (configurationValidationStatusChanged (bool)));
  connect (this, SIGNAL (configurationValidationStatus (QStringList)),
           main, SLOT (configurationValidationStatusChanged (QStringList)));
  connect (main, SIGNAL (applyCurrentConfiguration()),
           this, SLOT (applyCurrentConfiguration()));
  connect (main, SIGNAL (selectJointFromBodyName (std::string)),
           this, SLOT (selectJointFromBodyName (std::string)));
  connect (this, SIGNAL (logJobFailed(int,QString)),
           main, SLOT (logJobFailed(int, QString)));

  main->osg()->createGroup("joints");
  main->osg()->addToGroup("joints", "hpp-gui");
}

QString HppWidgetsPlugin::name() const
{
  return QString ("Widgets for hpp-corbaserver");
}

void HppWidgetsPlugin::loadRobotModel(DialogLoadRobot::RobotDefinition rd)
{
  client()->robot()->loadRobotModel(
        QSTRING_TO_CONSTCHARARRAY(rd.robotName_),
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

void HppWidgetsPlugin::openConnection ()
{
  closeConnection ();
  hpp_ = new hpp::corbaServer::Client (0,0);
  hpp_->connect ();
}

void HppWidgetsPlugin::closeConnection ()
{
  if (hpp_) delete hpp_;
  hpp_ = NULL;
}

void HppWidgetsPlugin::applyCurrentConfiguration()
{
  MainWindow * main = MainWindow::instance ();
  main->statusBar()->showMessage("Applying current configuration...");
  if (jointMap_.isEmpty()) {
      main->logError("The current configuration cannot be applied. This is probably because you are using external commands (python interface) and you did not refresh this GUI."
                     " Use the refresh button \"Tools\" menu.");
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
  for (std::list<std::string>::const_iterator it = jointFrames_.begin ();
       it != jointFrames_.end (); ++it) {
      std::string n = escapeJointName(*it);
      hpp::Transform__var t = client()->robot()->getJointPosition(it->c_str());
      for (size_t i = 0; i < 7; ++i) T[i] = (float)t[i];
      main->osg()->applyConfiguration (n.c_str (), T);
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
  QRegExp collision ("Collision between object (.*) and (.*)");
  QStringList col;
  if (!bb) {
      if (collision.exactMatch(QString::fromLocal8Bit(report))) {
          CORBA::String_var robotName = client ()->robot()->getRobotName();
          size_t pos = strlen(robotName) + 1;
          for (int i = 1; i < 3; ++i) {
              std::string c = collision.cap (i).toStdString();
              bool found = false;
              foreach (const JointElement& je, jointMap_) {
                  if (je.bodyName.length() <= pos)
                    continue;
                  size_t len = je.bodyName.length() - pos;
                  if (je.bodyName.compare(pos, len, c, 0, len) == 0) {
                      col.append(QString::fromStdString(je.bodyName));
                      found = true;
                      break;
                    }
                }
              if (!found) col.append(collision.cap (i));
            }
        } else {
          qDebug () << report;
          col.append(QString::fromLocal8Bit(client ()->robot()->getRobotName()));
        }
      qDebug () << col;
    }
  emit configurationValidationStatus (col);
}

void HppWidgetsPlugin::selectJointFromBodyName(const std::string &bodyName)
{
  boost::regex roadmap ("^(roadmap|path[0-9]+)_(.*)/(node|edge)([0-9]+)$");
  boost::cmatch what;
  if (boost::regex_match (bodyName.c_str(), what, roadmap)) {
      std::string group; group.assign(what[1].first, what[1].second);
      std::string joint; joint.assign(what[2].first, what[2].second);
      std::string type;  type .assign(what[3].first, what[3].second);
      int n = std::atoi (what[4].first);
      qDebug () << "Detected the" << group.c_str() << type.c_str() << n << "of joint" << joint.c_str();
      if (type == "node") {
          try {
            hpp::floatSeq_var q = hpp_->problem()->node(n);
            hpp_->robot()->setCurrentConfig(q.in());
            MainWindow::instance()->requestApplyCurrentConfiguration();
          } catch (const hpp::Error& e) {
            MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
          }
        } else if (type == "edge") {
          // TODO
        }
      return;
    }
  foreach (const JointElement& je, jointMap_) {
      if (bodyName.compare(je.bodyName) == 0) {
          // TODO: use je.item for a faster selection.
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
  a= new JointAction (tr("Set &bounds..."), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), jointTreeWidget_, SLOT (openJointBoundDialog(std::string)));
  l.append(a);
  a= new JointAction (tr("Add joint &frame"), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), SLOT (addJointFrame(std::string)));
  l.append(a);
  a = new JointAction (tr("Display &roadmap"), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), this, SLOT (displayRoadmap(std::string)));
  l.append(a);
  a = new JointAction (tr("Display &waypoints of selected path"), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), pathPlayer_, SLOT (displayWaypointsOfPath(std::string)));
  l.append(a);
  a = new JointAction (tr("Display selected &path"), jointName, 0);
  connect (a, SIGNAL (triggered(std::string)), pathPlayer_, SLOT (displayPath(std::string)));
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

std::string HppWidgetsPlugin::getSelectedJoint()
{
  return jointTreeWidget_->selectedJoint();
}

Roadmap* HppWidgetsPlugin::createRoadmap(const std::string &jointName)
{
  Roadmap* r = new Roadmap (this);
  r->initRoadmap(jointName);
  return r;
}

void HppWidgetsPlugin::displayRoadmap(const std::string &jointName)
{
  Roadmap* r = createRoadmap (jointName);
  r->displayRoadmap();
  delete r;
}

void HppWidgetsPlugin::addJointFrame (const std::string& jointName)
{
  MainWindow* main = MainWindow::instance ();
  std::string target = createJointGroup(jointName);
  const std::string n = target + "/XYZ";
  const float color[4] = {1,0,0,1};

  /// This returns false if the frame already exists
  if (main->osg()->addXYZaxis (n.c_str(), color, 0.005f, 1.f)) {
      main->osg()->setVisibility (n.c_str(), "ALWAYS_ON_TOP");
      return;
    } else {
      main->osg()->setVisibility (n.c_str(), "ALWAYS_ON_TOP");
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

std::string HppWidgetsPlugin::escapeJointName(const std::string jn)
{
  std::string target = jn;
  boost::replace_all (target, "/", "__");
  return target;
}

std::string HppWidgetsPlugin::createJointGroup(const std::string jn)
{
  MainWindow* main = MainWindow::instance ();
  std::string target = escapeJointName(jn);
  if (main->osg()->createGroup(target.c_str())) {
      main->osg()->addToGroup(target.c_str(), "joints");

      hpp::Transform__var t = client()->robot()->getJointPosition
          (jn.c_str());
      float p[7];
      for (std::size_t i = 0; i < 7; ++i) p[i] = (float)t[i];
      jointFrames_.push_back(jn);
      main->osg()->applyConfiguration (target.c_str(), p);
      main->osg()->refresh();
    }
  return target;
}

Q_EXPORT_PLUGIN2 (hppwidgetsplugin, HppWidgetsPlugin)
