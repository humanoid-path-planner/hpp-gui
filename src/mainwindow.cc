#include "hpp/gui/mainwindow.h"
#include "ui_mainwindow.h"

#include <gepetto/viewer/corba/server.hh>

#include "hpp/gui/windows-manager.h"
#include "hpp/gui/osgwidget.h"
#include "hpp/gui/tree-item.h"
#include "hpp/gui/dialog/dialogloadrobot.h"
#include "hpp/gui/dialog/dialogloadenvironment.h"
#include "hpp/gui/plugin-interface.h"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

MainWindow* MainWindow::instance_ = NULL;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui_(new Ui::MainWindow),
  centralWidget_ (),
  osgViewerManagers_ (WindowsManager::create()),
  osgServer_ (new ViewerServerProcess (
                new graphics::corbaServer::Server (osgViewerManagers_, 0, NULL, true))),
  backgroundQueue_(),
  worker_ ()
{
  MainWindow::instance_ = this;
  ui_->setupUi(this);

  osgServer_.start();
  // This scene contains elements required for User Interaction.
  osg()->createScene("hpp-gui");

  // Setup the tree views
  bodyTreeModel_  = new QStandardItemModel;
  ui_->bodyTree ->setModel(bodyTreeModel_ );
  ui_->bodyTree->setSelectionMode(QAbstractItemView::SingleSelection);

  // Setup the main OSG widget
  connect (this, SIGNAL (createView(QString)), SLOT (onCreateView()));

  connect (ui_->actionRefresh, SIGNAL (activated()), SLOT (requestRefresh()));
  connect (this, SIGNAL (refresh()), SLOT (reloadBodyTree()));

  connect (&backgroundQueue_, SIGNAL (done(int)), this, SLOT (handleWorkerDone(int)));
  connect (&backgroundQueue_, SIGNAL (failed(int,const QString&)),
           this, SLOT (logJobFailed(int, const QString&)));
  connect (this, SIGNAL (sendToBackground(WorkItem*)),
           &backgroundQueue_, SLOT (perform(WorkItem*)));
  connect (ui_->bodyTree->selectionModel(), SIGNAL (selectionChanged(QItemSelection,QItemSelection)),
           SLOT (bodySelectionChanged(QItemSelection,QItemSelection)));
  backgroundQueue_.moveToThread(&worker_);
  worker_.start();

  setupInterface();

  readSettings();
}

MainWindow::~MainWindow()
{
  writeSettings();
  worker_.quit();
  osgServer_.wait();
  worker_.wait();
  delete ui_;
}

MainWindow *MainWindow::instance()
{
  return instance_;
}

void MainWindow::insertDockWidget(QDockWidget *dock, Qt::DockWidgetArea area, Qt::Orientation orientation)
{
  addDockWidget(area, dock, orientation);
  dock->setVisible (false);
  dock->toggleViewAction ()->setIcon(QIcon::fromTheme("window-new"));
  ui_->menuWindow->addAction(dock->toggleViewAction ());
}

void MainWindow::removeDockWidget(QDockWidget *dock)
{
  ui_->menuWindow->removeAction(dock->toggleViewAction());
  QMainWindow::removeDockWidget(dock);
}

BackgroundQueue& MainWindow::worker()
{
  return backgroundQueue_;
}

WindowsManagerPtr_t MainWindow::osg() const
{
  return osgViewerManagers_;
}

OSGWidget *MainWindow::centralWidget() const
{
  return centralWidget_;
}

PluginManager *MainWindow::pluginManager()
{
  return &pluginManager_;
}

QItemSelectionModel *MainWindow::bodySelectionModel()
{
  return ui_->bodyTree->selectionModel();
}

void MainWindow::selectBodyByName(const QString &bodyName)
{
  QList<QStandardItem*> matches =
      bodyTreeModel_->findItems(bodyName,
                                Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchRecursive);
  if (matches.empty()) {
      qDebug () << "Body" << bodyName << "not found.";
      ui_->bodyTree->clearSelection();
    } else {
      ui_->bodyTree->setCurrentIndex(matches.first()->index());
    }
}

void MainWindow::bodySelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  QStringList sel, desel;
  foreach (QModelIndex index, selected.indexes()) {
      BodyTreeItem *item = dynamic_cast <BodyTreeItem*>
          (bodyTreeModel_->itemFromIndex(index));
      if (item) {
          const std::string& s = item->node()->getID();
          sel << QString::fromStdString(s);
          osg ()->setHighlight(s.c_str(), 2);
        }
    }
  foreach (QModelIndex index, deselected.indexes()) {
      BodyTreeItem *item = dynamic_cast <BodyTreeItem*>
          (bodyTreeModel_->itemFromIndex(index));
      if (item) {
          const std::string& s = item->node()->getID();
          desel << QString::fromStdString(s);
          osg ()->setHighlight(s.c_str(), 0);
        }
    }
  emit selectedBodyChanged (sel, desel);
}

void MainWindow::log(const QString &text)
{
  ui_->logText->insertHtml("<hr/><font color=black>"+text+"</font>");
}

void MainWindow::logError(const QString &text)
{
  if (!ui_->dockWidget_log->isVisible()) {
      ui_->dockWidget_log->show();
    }
  ui_->logText->insertHtml("<hr/><font color=red>"+text+"</font>");
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
  logError (QString ("Job ") + QString::number (id) + " failed: " + text);
}

OSGWidget *MainWindow::delayedCreateView(QString name)
{
  delayedCreateView_.lock();
  emit createView(name);
  delayedCreateView_.lock();
  delayedCreateView_.unlock();
  return osgWindows_.last();
}

void MainWindow::requestRefresh()
{
  emit refresh ();
}

void MainWindow::reloadBodyTree()
{
  bodyTreeModel_->clear();
  std::vector <std::string> sceneNames = osgViewerManagers_->getSceneList ();
  for (unsigned int i = 0; i < sceneNames.size(); ++i) {
      graphics::GroupNodePtr_t group = osgViewerManagers_->getScene(sceneNames[i]);
      if (!group) continue;
      addBodyToTree(group);
    }
}

OSGWidget *MainWindow::onCreateView()
{
  QString objName = "hpp_gui_window_" + QString::number(osgWindows_.size());
  OSGWidget* osgWidget = new OSGWidget (osgViewerManagers_, objName.toStdString(),
                                        this, centralWidget_, 0);
  if (!osgWindows_.empty()) {
      QDockWidget* dockOSG = new QDockWidget (
            tr("OSG Viewer") + " " + QString::number (osgWindows_.size()), this);
      osgWidget->setObjectName(objName);
      dockOSG->setWidget(osgWidget);
      addDockWidget(Qt::RightDockWidgetArea, dockOSG);
    } else {
      // This OSGWidget should be the central view
      centralWidget_ = osgWidget;
      centralWidget_->setObjectName(objName);
      setCentralWidget(centralWidget_);
      connect(ui_->actionHome, SIGNAL (activated()), centralWidget_, SLOT (onHome()));
      connect(ui_->actionSelection, SIGNAL (activated()), centralWidget_, SLOT (selectionMode()));
      connect(ui_->actionCamera_control_mode, SIGNAL (activated()), centralWidget_, SLOT (cameraManipulationMode()));
      ui_->osgToolBar->show();

      osg()->addSceneToWindow("hpp-gui", centralWidget_->windowID());
      osg()->addFloor("hpp-gui/floor");
    }
  osgWindows_.append(osgWidget);
  delayedCreateView_.unlock();
  return osgWidget;
}

void MainWindow::openLoadRobotDialog()
{
  statusBar()->showMessage("Loading a robot...");
  DialogLoadRobot* d = new DialogLoadRobot (this);
  if (d->exec () == QDialog::Accepted) {
      createCentralWidget();
      DialogLoadRobot::RobotDefinition rd = d->getSelectedRobotDescription();

      QDir dir (rd.packagePath_); dir.cd("urdf");
      QString urdfFile = dir.absoluteFilePath(rd.modelName_ + rd.urdfSuf_ + ".urdf");
      try {
        centralWidget_->loadURDF(rd.robotName_, urdfFile, rd.mesh_);
      } catch (std::runtime_error& exc) {
        logError (exc.what ());
      }
      robotNames_.append (rd.robotName_);

      QString what = QString ("Loading robot ") + rd.name_;
      WorkItem* item;
      foreach (ModelInterface* loader, pluginManager_.get <ModelInterface> ()) {
          item = new WorkItem_1 <ModelInterface, void,
              DialogLoadRobot::RobotDefinition>
              (loader, &ModelInterface::loadRobotModel, rd);
          logJobStarted(item->id(), what);
          emit sendToBackground(item);
        }
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
      createCentralWidget();
      DialogLoadEnvironment::EnvironmentDefinition ed = e->getSelectedDescription();

      QDir d (ed.packagePath_); d.cd("urdf");
      QString urdfFile = d.absoluteFilePath(ed.urdfFilename_ + ".urdf");
      try {
        osgViewerManagers_->addUrdfObjects(QSTRING_TO_CONSTCHARARRAY (ed.envName_),
                                           QSTRING_TO_CONSTCHARARRAY (urdfFile),
                                           QSTRING_TO_CONSTCHARARRAY (ed.mesh_),
                                           true);
        osgViewerManagers_->addSceneToWindow(QSTRING_TO_CONSTCHARARRAY (ed.envName_),
                                             centralWidget_->windowID());
      } catch (std::runtime_error& exc) {
        log (exc.what ());
      }
      addBodyToTree(osgViewerManagers_->getScene(ed.envName_.toStdString()));

      QString what = QString ("Loading environment ") + ed.name_;
      WorkItem* item;
      foreach (ModelInterface* loader, pluginManager_.get <ModelInterface> ()) {
          item = new WorkItem_1 <ModelInterface, void,
              DialogLoadEnvironment::EnvironmentDefinition>
              (loader, &ModelInterface::loadEnvironmentModel, ed);
          logJobStarted(item->id(), what);
          emit sendToBackground(item);
        }
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
          OSGWidget* w = (OSGWidget*)toDo->userData(0);
          if (!w) return;
          osgViewerManagers_->addSceneToWindow(item->node()->getID().c_str(),
                                               w->windowID());
        }
      return;
    }
}

void MainWindow::handleWorkerDone(int /*id*/)
{
}

void MainWindow::setupInterface()
{
  // Menu "Window"
  QMenu* toolbar = ui_->menuWindow->addMenu("Tool bar");
  toolbar->setIcon(QIcon::fromTheme("configure-toolbars"));
  ui_->mainToolBar->setVisible(false);
  ui_->osgToolBar->setVisible(false);
  toolbar->addAction (ui_->mainToolBar->toggleViewAction ());
  toolbar->addAction (ui_->osgToolBar->toggleViewAction ());

  ui_->menuWindow->addSeparator();

  ui_->dockWidget_bodyTree->setVisible (false);
  ui_->dockWidget_bodyTree->toggleViewAction ()->setIcon(QIcon::fromTheme("window-new"));
  ui_->menuWindow->addAction(ui_->dockWidget_bodyTree->toggleViewAction ());
  ui_->dockWidget_log->setVisible (false);
  ui_->dockWidget_log->toggleViewAction ()->setIcon(QIcon::fromTheme("window-new"));
  ui_->menuWindow->addAction(ui_->dockWidget_log->toggleViewAction ());

  ui_->menuWindow->addSeparator();

  // Setup the status bar
  collisionIndicator_ = new LedIndicator (statusBar());
  statusBar()->addPermanentWidget(collisionIndicator_);
}

void MainWindow::createCentralWidget()
{
  if (!osgWindows_.empty()) return;
  onCreateView();
}

void MainWindow::readSettings()
{
  do {
      QSettings robot (QSettings::SystemScope,
                       QCoreApplication::organizationName(), "robots", this);
      if (robot.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + robot.fileName());
          break;
        } else {
          foreach (QString name, robot.childGroups()) {
              robot.beginGroup(name);
              QString robotName = robot.value("RobotName", name).toString();
              QDir packagePath (robot.value("PackagePath", "").toString());
              QString meshDirectory;
              if (packagePath.exists()) {
                  QDir meshDir = packagePath; meshDir.cdUp();
                  meshDirectory = robot.value("MeshDirectory", meshDir.absolutePath()).toString();
                } else {
                  meshDirectory = robot.value("MeshDirectory", "").toString();
                }
              DialogLoadRobot::addRobotDefinition(
                    name,
                    robotName,
                    robot.value("RootJointType", "freeflyer").toString(),
                    robot.value("ModelName", robotName).toString(),
                    robot.value("Package", packagePath.dirName()).toString(),
                    packagePath.path(),
                    robot.value("URDFSuffix", "").toString(),
                    robot.value("SRDFSuffix", "").toString(),
                    meshDirectory
                    );
              robot.endGroup();
            }
          log(QString ("Read configuration file ") + robot.fileName());
        }
    } while (0);
  do {
      QSettings env (QSettings::SystemScope,
                     QCoreApplication::organizationName(), "environments", this);
      if (env.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + env.fileName());
          break;
        } else {
          foreach (QString name, env.childGroups()) {
              env.beginGroup(name);
              QString envName = env.value("EnvironmentName", name).toString();
              QDir packagePath (env.value("PackagePath", "").toString());
              QString meshDirectory;
              if (packagePath.exists()) {
                  QDir meshDir = packagePath; meshDir.cdUp();
                  meshDirectory = env.value("MeshDirectory", meshDir.absolutePath()).toString();
                } else {
                  meshDirectory = env.value("MeshDirectory", "").toString();
                }
              DialogLoadEnvironment::addEnvironmentDefinition(
                    name,
                    envName,
                    env.value("Package", packagePath.dirName()).toString(),
                    packagePath.path(),
                    env.value("URDFFilename").toString(),
                    meshDirectory
                    );
              env.endGroup();
            }
          log (QString ("Read configuration file ") + env.fileName());
        }
    } while (0);
  do {
      QSettings env (QSettings::SystemScope,
                     QCoreApplication::organizationName(), "settings", this);
      if (env.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + env.fileName());
          break;
        } else {
          env.beginGroup("plugins");
          foreach (QString name, env.childKeys()) {
              pluginManager_.add(name, this, env.value(name, true).toBool());
          }
          log (QString ("Read configuration file ") + env.fileName());
        }
    } while (0);
}

void MainWindow::writeSettings()
{
  do {
      QSettings robot (QSettings::SystemScope,
                       QCoreApplication::organizationName(), "robots", this);
      if (!robot.isWritable()) {
          log (QString("Configuration file ") + robot.fileName() + QString(" is not writable."));
          break;
        }
      foreach (DialogLoadRobot::RobotDefinition rd, DialogLoadRobot::getRobotDefinitions()) {
          if (rd.name_.isEmpty()) continue;
          robot.beginGroup(rd.name_);
          robot.setValue("RobotName", rd.robotName_);
          robot.setValue("ModelName", rd.modelName_);
          robot.setValue("RootJointType", rd.rootJointType_);
          robot.setValue("Package", rd.package_);
          robot.setValue("PackagePath", rd.packagePath_);
          robot.setValue("URDFSuffix", rd.urdfSuf_);
          robot.setValue("SRDFSuffix", rd.srdfSuf_);
          robot.setValue("MeshDirectory", rd.mesh_);
          robot.endGroup();
        }
      log (QString("Wrote configuration file ") + robot.fileName());
    } while (0);
  do {
      QSettings env (QSettings::SystemScope,
                     QCoreApplication::organizationName(), "environments", this);
      if (!env.isWritable()) {
          log(QString ("Configuration file") + env.fileName() + QString("is not writable."));
          break;
        }
      foreach (DialogLoadEnvironment::EnvironmentDefinition ed, DialogLoadEnvironment::getEnvironmentDefinitions()) {
          if (ed.name_.isEmpty()) continue;
          env.beginGroup(ed.name_);
          env.setValue("RobotName", ed.envName_);
          env.setValue("Package", ed.package_);
          env.setValue("PackagePath", ed.packagePath_);
          env.setValue("URDFFilename", ed.urdfFilename_);
          env.setValue("MeshDirectory", ed.mesh_);
          env.endGroup();
        }
      log (QString ("Wrote configuration file ") + env.fileName());
    } while (0);
  do {
      QSettings env (QSettings::SystemScope,
                     QCoreApplication::organizationName(), "settings", this);
      if (env.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + env.fileName());
          break;
        } else {
          env.beginGroup("plugins");
          for (PluginManager::Map::const_iterator p = pluginManager_.plugins ().constBegin();
            p != pluginManager_.plugins().constEnd(); p++) {
            env.setValue(p.key(), p.value()->isLoaded());
          }
          log (QString ("Read configuration file ") + env.fileName());
        }
    } while (0);
}

void MainWindow::addBodyToTree(graphics::GroupNodePtr_t group)
{
  bodyTreeModel_->appendRow(new BodyTreeItem (group));
}

void MainWindow::requestApplyCurrentConfiguration()
{
  emit applyCurrentConfiguration();
  requestConfigurationValidation();
}

void MainWindow::requestConfigurationValidation()
{
  emit configurationValidation();
}

void MainWindow::onOpenPluginManager()
{
  PluginManagerDialog d (&pluginManager_, this);
  d.exec ();
}

void MainWindow::configurationValidationStatusChanged (bool valid)
{
  collisionIndicator_->switchLed (valid);
  int state = (valid)?0:1;
  foreach(const QString& s, robotNames_) {
      osg ()->setHighlight(s.toLocal8Bit().data(), state);
    }
  if (!valid) {
      log ("Current configuration is NOT valid.");
    }
}

void MainWindow::requestSelectJointFromBodyName(const std::string &bodyName)
{
  selectBodyByName (QString::fromStdString(bodyName));
  emit selectJointFromBodyName(bodyName);
}
