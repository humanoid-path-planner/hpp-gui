#include "hpp/gui/mainwindow.h"
#include "ui_mainwindow.h"

#include <hpp/core/problem-solver.hh>
#include <hpp/corbaserver/server.hh>
#include <hpp/corbaserver/client.hh>
#include <gepetto/viewer/corba/server.hh>

#include "hpp/gui/osgwidget.h"
#include "hpp/gui/solverwidget.h"
#include "hpp/gui/pathplayer.h"
#include "hpp/gui/tree-item.h"
#include "hpp/gui/dialog/dialogloadrobot.h"
#include "hpp/gui/dialog/dialogloadenvironment.h"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

MainWindow* MainWindow::instance_ = NULL;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui_(new Ui::MainWindow),
  centralWidget_ (),
  problemSolver_ (new hpp::core::ProblemSolver),
  osgViewerManagers_ (graphics::WindowsManager::create()),
  hppServer_ (new HppServerProcess (
                new hpp::corbaServer::Server (problemSolver_, 0, NULL, true))),
  osgServer_ (new ViewerServerProcess (
                new graphics::corbaServer::Server (osgViewerManagers_, 0, NULL, true))),
  hppClient_ (new hpp::corbaServer::Client (0, 0)),
  backgroundQueue_(),
  worker_ ()
{
  MainWindow::instance_ = this;
  ui_->setupUi(this);

  findChild <PathPlayer*> ("dockWidgetContents_player")->setup ();
  solver()->setup ();
  hppServer_.start();
  osgServer_.start();

  bodyTreeModel_  = new QStandardItemModel;
  jointTreeModel_ = new QStandardItemModel;
  ui_->bodyTree ->setModel(bodyTreeModel_ );
  ui_->jointTree->setModel(jointTreeModel_);
  ui_->jointTree->setItemDelegate (new JointItemDelegate(this));
  ui_->jointTree->header()->setVisible(true);
  QStringList l; l << "Joint" << "Lower bound" << "Upper bound";
  jointTreeModel_->setHorizontalHeaderLabels(l);
  jointTreeModel_->setColumnCount(3);

  QString objName = "hpp_gui_main_window";
  centralWidget_ = new OSGWidget (osgViewerManagers_, objName.toStdString(), this, 0, 0);
  centralWidget_->setObjectName(objName);
  setCentralWidget(centralWidget_);
  osgWindows_.append(centralWidget_);
  connect(ui_->actionHome, SIGNAL (activated()), centralWidget_, SLOT (onHome()));
  hppClient_->connect();

  connect (&backgroundQueue_, SIGNAL (done(int)), this, SLOT (handleWorkerDone(int)));
  connect (&backgroundQueue_, SIGNAL (failed(int,const QString&)),
           this, SLOT (logJobFailed(int, const QString&)));
  connect (this, SIGNAL (sendToBackground(WorkItem*)),
           &backgroundQueue_, SLOT (perform(WorkItem*)));
  backgroundQueue_.moveToThread(&worker_);
  worker_.start();

  // Group dock widgets
  this->tabifyDockWidget(ui_->dockWidget_jointTree, ui_->dockWidget_bodyTree);
  // Menu "Window"
  ui_->dockWidget_bodyTree->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_bodyTree->toggleViewAction ());
  ui_->dockWidget_jointTree->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_jointTree->toggleViewAction ());
  ui_->dockWidget_configurations->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_configurations->toggleViewAction ());
  ui_->dockWidget_solver->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_solver->toggleViewAction ());
  ui_->dockWidget_player->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_player->toggleViewAction ());
  ui_->dockWidget_log->setVisible (false);
  ui_->menuWindow->addAction(ui_->dockWidget_log->toggleViewAction ());
  ui_->menuWindow->addSeparator();
  QMenu* toolbar = ui_->menuWindow->addMenu("Tool bar");
  ui_->mainToolBar->setVisible(false);
  ui_->osgToolBar->setVisible(false);
  toolbar->addAction (ui_->mainToolBar->toggleViewAction ());
  toolbar->addAction (ui_->osgToolBar->toggleViewAction ());

  // Setup the status bar
  collisionIndicator_ = new LedIndicator (statusBar());
  statusBar()->addPermanentWidget(collisionIndicator_);
}

MainWindow::~MainWindow()
{
  worker_.quit();
  hppServer_.wait();
  osgServer_.wait();
  worker_.wait();
  delete hppClient_;
  delete ui_;
}

MainWindow *MainWindow::instance()
{
  return instance_;
}

CorbaServer &MainWindow::hppServer()
{
  return hppServer_;
}

hpp::corbaServer::Client *MainWindow::hppClient()
{
  return hppClient_;
}

BackgroundQueue& MainWindow::worker()
{
  return backgroundQueue_;
}

SolverWidget *MainWindow::solver() const
{
  return ui_->dockWidgetContents_solver;
}

graphics::WindowsManagerPtr_t MainWindow::osg() const
{
  return osgViewerManagers_;
}

OSGWidget *MainWindow::centralWidget() const
{
  return centralWidget_;
}

void MainWindow::log(const QString &text)
{
  ui_->logText->appendPlainText (text);
}

void MainWindow::emitSendToBackground(WorkItem *item)
{
  emit sendToBackground(item);
}

void MainWindow::logJobStarted(int id, const QString &text)
{
  log (QString ("Starting job ") + QString::number (id) + ": " + text);
}

void MainWindow::logJobDone(int id, const QString &text)
{
  log (QString ("Job ") + QString::number (id) + " done: " + text);
}

void MainWindow::logJobFailed(int id, const QString &text)
{
  log (QString ("Job ") + QString::number (id) + " failed: " + text);
}

void MainWindow::onCreateView()
{
  QDockWidget* dockOSG = new QDockWidget (
        tr("OSG Viewer") + " " + QString::number (osgWindows_.size()), this);
  QString objName = "hpp_gui_side_window_" + QString::number(osgWindows_.size());
  OSGWidget* osgWidget = new OSGWidget(osgViewerManagers_,
                                       objName.toStdString(),
                                       dockOSG, centralWidget_);
  osgWidget->setObjectName(objName);
  osgWindows_.append(osgWidget);
  dockOSG->setWidget(osgWidget);
  addDockWidget(Qt::RightDockWidgetArea, dockOSG);
}

void MainWindow::openLoadRobotDialog()
{
  statusBar()->showMessage("Loading a robot...");
  DialogLoadRobot* d = new DialogLoadRobot (this);
  if (d->exec () == QDialog::Accepted) {
      LoadRobot* lr = new LoadRobot;
      LoadRobot& loadDone = *lr;
      loader_.push_back(lr);
      loadDone.rd = d->getSelectedRobotDescription();
      DialogLoadRobot::RobotDefinition& rd = loadDone.rd;
      loadDone.name_  = rd.name_.toStdString();
      loadDone.urdfSuf_ = rd.urdfSuf_.toStdString();
      loadDone.srdfSuf_ = rd.srdfSuf_.toStdString();
      loadDone.package_ = rd.package_.toStdString();
      loadDone.modelName_ = rd.modelName_.toStdString();
      loadDone.rootJointType_ = rd.rootJointType_.toStdString();
      loadDone.what = QString ("Loading robot ") + rd.name_;
      WorkItem* item = new WorkItem_6 <
          hpp::corbaserver::_objref_Robot, void,
          const char*, const char*, const char*, const char*, const char*, const char*>
          (hppClient()->robot (), &hpp::corbaserver::_objref_Robot::loadRobotModel,
           STDSTRING_TO_CONSTCHARARRAY(loadDone.name_),
           STDSTRING_TO_CONSTCHARARRAY(loadDone.rootJointType_),
           STDSTRING_TO_CONSTCHARARRAY(loadDone.package_),
           STDSTRING_TO_CONSTCHARARRAY(loadDone.modelName_),
           STDSTRING_TO_CONSTCHARARRAY(loadDone.urdfSuf_),
           STDSTRING_TO_CONSTCHARARRAY(loadDone.srdfSuf_));
      loadDone.id = item->id();
      logJobStarted(loadDone.id, loadDone.what);
      emit sendToBackground(item);
      QDir dir (rd.packagePath_); dir.cd("urdf");
      QString urdfFile = dir.absoluteFilePath(rd.modelName_ + rd.urdfSuf_ + ".urdf");
      centralWidget_->loadURDF(rd.robotName_, urdfFile, rd.mesh_);
    }
  d->close();
  statusBar()->clearMessage();
  d->deleteLater();
}

void MainWindow::openLoadEnvironmentDialog()
{
  statusBar()->showMessage("Loading an environment...");
  DialogLoadEnvironment* e = new DialogLoadEnvironment (this);
  if (e->exec() == QDialog::Accepted) {
      LoadEnvironment* le = new LoadEnvironment;
      loader_.push_back(le);
      DialogLoadEnvironment::EnvironmentDefinition rd = e->getSelectedDescription();
      le->prefix_ = rd.envName_.toStdString() + ("/");
      le->urdfFilename_ = rd.urdfFilename_.toStdString();
      le->package_ = rd.package_.toStdString();
      le->what = QString ("Loading environment ") + rd.name_;
      WorkItem* item = new WorkItem_3 <
          hpp::corbaserver::_objref_Obstacle, void,
          const char*, const char*, const char*>
          (hppClient()->obstacle(), &hpp::corbaserver::_objref_Obstacle::loadObstacleModel,
           STDSTRING_TO_CONSTCHARARRAY(le->package_),
           STDSTRING_TO_CONSTCHARARRAY(le->urdfFilename_),
           STDSTRING_TO_CONSTCHARARRAY(le->prefix_));
      le->id = item->id();
      logJobStarted(le->id, le->what);
      emit sendToBackground(item);
      QDir d (rd.packagePath_); d.cd("urdf");
      QString urdfFile = d.absoluteFilePath(rd.urdfFilename_ + ".urdf");
      osgViewerManagers_->addUrdfObjects(QSTRING_TO_CONSTCHARARRAY (rd.envName_),
                                         QSTRING_TO_CONSTCHARARRAY (urdfFile),
                                         QSTRING_TO_CONSTCHARARRAY (rd.mesh_),
                                         true);
      osgViewerManagers_->addSceneToWindow(QSTRING_TO_CONSTCHARARRAY (rd.envName_),
                                           centralWidget_->windowID());
      addBodyToTree(osgViewerManagers_->getScene(rd.envName_.toStdString()));
    }
  statusBar()->clearMessage();
  e->close();
  e->deleteLater();
}

void MainWindow::updateBodyTree(const QModelIndex &index)
{
  VisibilityItem* vi = dynamic_cast <VisibilityItem*> (bodyTreeModel_->itemFromIndex(index));
  if (vi) vi->update();
}

void MainWindow::updateJointTree(const QModelIndex &index)
{
}

void MainWindow::showTreeContextMenu(const QPoint &point)
{
  QMenu contextMenu (tr("Node"), this);
  QModelIndex index = ui_->bodyTree->indexAt(point);
  if(index.isValid()) {
      BodyTreeItem *item =
          dynamic_cast <BodyTreeItem*> (bodyTreeModel_->itemFromIndex(index));
      if (!item) return;
      QMenu* viewmode = contextMenu.addMenu(tr("Viewing mode"));
      QMenu* windows = contextMenu.addMenu(tr("Attach to window"));
      /*QAction* fill = */viewmode->addAction ("FILL");
      /*QAction* wireframe = */viewmode->addAction ("WIREFRAME");
      /*QAction* f_and_w = */viewmode->addAction ("FILL_AND_WIREFRAME");
      foreach (OSGWidget* w, osgWindows_) {
          QAction* aw = windows->addAction(w->objectName());
          aw->setUserData(0, (QObjectUserData*)w);
        }
      QAction* toDo = contextMenu.exec(ui_->bodyTree->mapToGlobal(point));
      if (!toDo) return;
      if (toDo->parent() == viewmode)
        osgViewerManagers_->setWireFrameMode(
              item->node()->getID().c_str(),
              QSTRING_TO_CONSTCHARARRAY (toDo->text()));
      else if (toDo->parent() == windows) {
          OSGWidget* w = dynamic_cast <OSGWidget*> (toDo->userData(0));
          if (!w) return;
          osgViewerManagers_->addSceneToWindow(item->node()->getID().c_str(),
                                               w->windowID());
        }
    }
}

void MainWindow::handleWorkerDone(int id)
{
  foreach (LoadDoneStruct* d, loader_) {
      if (d->is (id)) {
          d->done();
          loader_.removeOne(d);
          delete d;
          return;
        }
    }
}

void MainWindow::computeObjectPosition()
{
  hpp::Names_t_var obs = hppClient()->obstacle()->getObstacleNames (true, false);
  hpp::Transform__out cfg = hpp::Transform__alloc () ;
  float d[7];
  for (size_t i = 0; i < obs->length(); ++i) {
      hppClient()->obstacle()->getObstaclePosition (obs[i], cfg);
      for (size_t j = 0; j < 7; j++) d[j] = cfg[j];
      const char* name = obs[i];
      osg ()->applyConfiguration(name, d);
    }
  osg()->refresh();
  delete cfg;
}

void MainWindow::addBodyToTree(graphics::GroupNodePtr_t group)
{
  bodyTreeModel_->appendRow(new BodyTreeItem (group));
}

void MainWindow::addJointToTree(const std::string name, JointTreeItem* parent)
{
  graphics::NodePtr_t node = osgViewerManagers_->getScene(jointsToLinkMap_[name]);
  hpp::floatSeq_var c = hppClient()->robot ()->getJointConfig (name.c_str());
  CORBA::Short nbDof = hppClient()->robot ()->getJointNumberDof (name.c_str());
  hpp::corbaserver::jointBoundSeq_var b = hppClient()->robot ()->getJointBounds (name.c_str());

//  char* n = new char[sizeof(char)*(name.length()+1)];
//  strcpy (n, name.c_str());
  JointTreeItem* j = new JointTreeItem (name.c_str(), c.in(), b.in(), nbDof, node);
  if (parent) {
      parent->appendRow(j);
    } else {
      jointTreeModel_->appendRow(j);
    }
  hpp::Names_t_var children = hppClient()->robot ()->getChildJointNames (name.c_str());
  for (size_t i = 0; i < children->length(); ++i)
    addJointToTree(std::string(children[i]),j);
}

void MainWindow::updateRobotJoints(const QString robotName)
{
  hpp::Names_t* joints = hppClient()->robot()->getAllJointNames ();
  for (size_t i = 0; i < joints->length (); ++i) {
      const char* jname = (*joints)[i];
      std::string linkName = robotName.toStdString() + "/" + std::string (hppClient()->robot()->getLinkName (jname));
      jointsToLink_.append(JointLinkPair(jname, linkName));
      jointsToLinkMap_[jname] = linkName;
    }
}

void MainWindow::applyCurrentConfiguration()
{
  statusBar()->showMessage("Applying current configuration...");
  foreach (JointLinkPair p, jointsToLink_) {
      hpp::Transform__slice* t = hppClient()->robot()->getLinkPosition(p.first.c_str());
      float T[7];
      for (size_t i = 0; i < 7; ++i) T[i]=t[i];
      osgViewerManagers_->applyConfiguration(p.second.c_str(), T);
    }
  osgViewerManagers_->refresh();
  requestConfigurationValidation();
  statusBar()->clearMessage();
}

void MainWindow::requestConfigurationValidation()
{
  hpp::floatSeq_var q = hppClient()->robot()->getCurrentConfig ();
  bool bb = false;
  CORBA::Boolean_out b = bb;
  hppClient()->robot()->isConfigValid (q.in(), b);
  collisionIndicator_->switchLed(b);
  if (!b) log ("Current configuration is NOT valid.");
}

bool MainWindow::close()
{
//  if (hpp_.isRunning()) hpp_.requestInterruption ();
//  if (worker_.isRunning()) worker_.requestInterruption ();
//  if (hpp_.isRunning()) hpp_.wait();
//  if (worker_.isRunning()) worker_.wait ();

  return QMainWindow::close();
}

void MainWindow::LoadRobot::done()
{
  std::string bjn;
  if (rd.rootJointType_.compare("freeflyer") == 0)   bjn = "base_joint_xyz";
  else if (rd.rootJointType_.compare("planar") == 0) bjn = "base_joint_xy";
  else if (rd.rootJointType_.compare("anchor") == 0) bjn = "base_joint";
  parent->updateRobotJoints(rd.robotName_);
  parent->addJointToTree(bjn, 0);
  parent->applyCurrentConfiguration();
  LoadDoneStruct::done();
}

void MainWindow::LoadEnvironment::done()
{
  parent->computeObjectPosition();
  LoadDoneStruct::done();
}

void MainWindow::LoadDoneStruct::done()
{
  parent->logJobDone(id, what);
  invalidate ();
}
