#include "hpp/gui/mainwindow.hh"
#include "ui_mainwindow.h"

#include <gepetto/viewer/corba/server.hh>

#include "hpp/gui/windows-manager.hh"
#include "hpp/gui/osgwidget.hh"
#include "hpp/gui/tree-item.hh"
#include "hpp/gui/dialog/dialogloadrobot.hh"
#include "hpp/gui/dialog/dialogloadenvironment.hh"
#include "hpp/gui/plugin-interface.hh"

#include <hpp/gui/meta.hh>

namespace hpp {
  namespace gui {
    MainWindow* MainWindow::instance_ = NULL;

    MainWindow::MainWindow(Settings settings, QWidget *parent) :
      QMainWindow(parent),
      settings_ (settings),
      ui_(new ::Ui::MainWindow),
      centralWidget_ (),
      osgViewerManagers_ (WindowsManager::create()),
      osgServer_ (NULL),
      backgroundQueue_(),
      worker_ ()
    {
      MainWindow::instance_ = this;
      ui_->setupUi(this);

      if (settings_.startGepettoCorbaServer) {
        osgServer_ = new CorbaServer (new ViewerServerProcess (
              new graphics::corbaServer::Server (
                osgViewerManagers_, 0, NULL, true)));
        osgServer_->start();
      }
      // This scene contains elements required for User Interaction.
      osg()->createScene("hpp-gui");

      // Setup the body tree view
      ui_->bodyTreeContent->init(ui_->bodyTree, ui_->toolBox);

      // Setup the main OSG widget
      connect (this, SIGNAL (createView(QString)), SLOT (onCreateView()));

      connect (ui_->actionRefresh, SIGNAL (triggered()), SLOT (requestRefresh()));

      connect (&backgroundQueue_, SIGNAL (done(int)), this, SLOT (handleWorkerDone(int)));
      connect (&backgroundQueue_, SIGNAL (failed(int,const QString&)),
          this, SLOT (logJobFailed(int, const QString&)));
      connect (this, SIGNAL (sendToBackground(WorkItem*)),
          &backgroundQueue_, SLOT (perform(WorkItem*)));
      backgroundQueue_.moveToThread(&worker_);
      worker_.start();

      setupInterface();

      readSettings();
    }

    MainWindow::~MainWindow()
    {
      if (settings_.autoWriteSettings)
        writeSettings();
      worker_.quit();
      if (osgServer_ != NULL) {
        osgServer_->wait();
        delete osgServer_;
      }
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

    BodyTreeWidget *MainWindow::bodyTree() const
    {
      return ui_->bodyTreeContent;
    }

    QList<OSGWidget *> MainWindow::osgWindows() const
    {
      return osgWindows_;
    }

    PluginManager *MainWindow::pluginManager()
    {
      return &pluginManager_;
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
      QScrollBar* sb = ui_->logText->verticalScrollBar();
      bool SBwasAtBottom = sb->value() == sb->maximum();
      ui_->logText->insertHtml("<hr/><font color=red>"+text+"</font>");
      if (SBwasAtBottom)
        sb->setValue(sb->maximum());
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

    OSGWidget *MainWindow::onCreateView()
    {
      QString objName = "hpp_gui_window_" + QString::number(osgWindows_.size());
      OSGWidget* osgWidget = new OSGWidget (osgViewerManagers_, objName.toStdString(),
          this, 0);
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
        connect(ui_->actionHome, SIGNAL (triggered()), centralWidget_, SLOT (onHome()));
        connect(ui_->actionSelection, SIGNAL (triggered()), centralWidget_, SLOT (selectionMode()));
        connect(ui_->actionCamera_control_mode, SIGNAL (triggered()), centralWidget_, SLOT (cameraManipulationMode()));
        ui_->osgToolBar->show();

        osg()->addSceneToWindow("hpp-gui", centralWidget_->windowID());
        connect(ui_->actionAdd_floor, SIGNAL (triggered()), centralWidget_, SLOT (addFloor()));
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
          osgViewerManagers_->addUrdfObjects(
              Traits<QString>::to_corba(ed.envName_).in(),
              Traits<QString>::to_corba(urdfFile   ).in(),
              Traits<QString>::to_corba(ed.mesh_   ).in(),
              true);
          osgViewerManagers_->addSceneToWindow(
              Traits<QString>::to_corba(ed.envName_).in(),
              centralWidget_->windowID());
        } catch (std::runtime_error& exc) {
          log (exc.what ());
        }
        bodyTree()->addBodyToTree(osgViewerManagers_->getScene(ed.envName_.toStdString()));

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

    void MainWindow::handleWorkerDone(int /*id*/)
    {
    }

    void MainWindow::resetConnection()
    {
      foreach (CorbaInterface* e, pluginManager_.get <CorbaInterface> ()) {
        e->openConnection ();
      }
    }

    void MainWindow::about()
    {
      QString devString;
      devString = trUtf8("<p>Version %1. For more information visit <a href=\"%2\">%3</a></p>"
          "<p><small>Copyright (c) 2015 CNRS<br/>By Joseph Mirabel and others.</small></p>"
          "<p><small>"
          "hpp-gui is free software: you can redistribute it and/or modify it under the "
          "terms of the GNU Lesser General Public License as published by the Free "
          "Software Foundation, either version 3 of the License, or (at your option) "
          "any later version.<br/><br/>"
          "hpp-gui is distributed in the hope that it will be "
          "useful, but WITHOUT ANY WARRANTY; without even the implied warranty "
          "of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
          "General Lesser Public License for more details.  You should have "
          "received a copy of the GNU Lesser General Public License along with hpp-gui."
          "If not, see <a href=\"http://www.gnu.org/licenses\">http://www.gnu.org/licenses<a/>."
          "</small></p>"
          )
        .arg(QApplication::applicationVersion())
        .arg(QApplication::organizationDomain())
        .arg(QApplication::organizationDomain());

      QMessageBox::about(this, QApplication::applicationName(), devString);
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
      ui_->dockWidget_bodyTree->toggleViewAction ()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_B);
      ui_->menuWindow->addAction(ui_->dockWidget_bodyTree->toggleViewAction ());
      ui_->dockWidget_log->setVisible (false);
      ui_->dockWidget_log->toggleViewAction ()->setIcon(QIcon::fromTheme("window-new"));
      ui_->dockWidget_log->toggleViewAction ()->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_L);
      ui_->menuWindow->addAction(ui_->dockWidget_log->toggleViewAction ());

      ui_->menuWindow->addSeparator();

      // Setup the status bar
      collisionIndicator_ = new LedIndicator (statusBar());
      collisionValidationActivated_ = new QCheckBox ();
      collisionValidationActivated_->setToolTip (tr("Automatically validate configurations."));
      collisionValidationActivated_->setCheckState (Qt::Checked);
      statusBar()->addPermanentWidget(collisionValidationActivated_);
      statusBar()->addPermanentWidget(collisionIndicator_);

      connect (collisionIndicator_, SIGNAL (mouseClickEvent()), SLOT(requestConfigurationValidation()));
      connect (ui_->actionAbout, SIGNAL (triggered ()), SLOT(about()));
      connect (ui_->actionReconnect, SIGNAL (triggered ()), SLOT(resetConnection()));
      connect (ui_->actionFetch_configuration, SIGNAL (triggered ()), SLOT(requestApplyCurrentConfiguration()));
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
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.predifinedRobotConf),
            this);
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
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.predifinedEnvConf),
            this);
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
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.configurationFile),
            this);
        if (env.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + env.fileName());
          break;
        } else {
          env.beginGroup("plugins");
          foreach (QString name, env.childKeys()) {
            pluginManager_.add(name, this,
                (settings_.noPlugin)?false:env.value(name, true).toBool()
                );
          }
          env.endGroup ();
          log (QString ("Read configuration file ") + env.fileName());
        }
      } while (0);
    }

    void MainWindow::writeSettings()
    {
      do {
        QSettings robot (QSettings::SystemScope,
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.predifinedRobotConf),
            this);
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
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.predifinedEnvConf),
            this);
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
            QCoreApplication::organizationName(),
            QString::fromStdString(settings_.configurationFile),
            this);
        if (env.status() != QSettings::NoError) {
          logError(QString ("Enable to open configuration file ") + env.fileName());
          break;
        } else {
          env.beginGroup("plugins");
          for (PluginManager::Map::const_iterator p = pluginManager_.plugins ().constBegin();
              p != pluginManager_.plugins().constEnd(); p++) {
            env.setValue(p.key(),
                (settings_.noPlugin)?false:p.value()->isLoaded());
          }
          env.endGroup ();
          log (QString ("Read configuration file ") + env.fileName());
        }
      } while (0);
    }

    void MainWindow::requestApplyCurrentConfiguration()
    {
      emit applyCurrentConfiguration();
      if (collisionValidationActivated_->isChecked ())
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
    }

    void MainWindow::configurationValidationStatusChanged (QStringList bodiesInCollision)
    {
      QStringList lastBodiesInCollision = lastBodiesInCollision_;
      lastBodiesInCollision_.clear();
      collisionIndicator_->switchLed (bodiesInCollision.empty());
      foreach (QString b, lastBodiesInCollision) {
        if (bodiesInCollision.removeAll(b) == 0) {
          /// This body is not in collision
          osg ()->setHighlight(b.toLocal8Bit().data(), 0);
        } else {
          /// This body is still in collision
          lastBodiesInCollision_.append(b);
        }
      }
      QString tooltip ("Collision between ");
      tooltip += bodiesInCollision.join (", ");
      foreach(const QString& b, bodiesInCollision) {
        osg ()->setHighlight(b.toLocal8Bit().data(), 1);
        lastBodiesInCollision_.append(b);
      }
      collisionIndicator_->setToolTip (tooltip);
    }

    void MainWindow::requestSelectJointFromBodyName(const std::string &bodyName)
    {
      ui_->bodyTreeContent->selectBodyByName (QString::fromStdString(bodyName));
      emit selectJointFromBodyName(bodyName);
    }
  } // namespace gui
} // namespace hpp
