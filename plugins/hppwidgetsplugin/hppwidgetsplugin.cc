#include "hppwidgetsplugin/hppwidgetsplugin.hh"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <QDockWidget>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>
#include <gepetto/gui/omniorb/url.hh>
#include <gepetto/gui/action-search-bar.hh>

#include <omniORB4/CORBA.h>

#include "hppwidgetsplugin/pathplayer.hh"
#include "hppwidgetsplugin/solverwidget.hh"
#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppwidgetsplugin/configurationlistwidget.hh"
#include "hppwidgetsplugin/joint-tree-item.hh"
#include "hppwidgetsplugin/constraintwidget.hh"
#include "hppwidgetsplugin/twojointsconstraint.hh"
#include "hppwidgetsplugin/listjointconstraint.hh"
#include "hppwidgetsplugin/conversions.hh"
#include "hppwidgetsplugin/joint-action.hh"

#include "hppwidgetsplugin/roadmap.hh"
#include <gepetto/gui/meta.hh>

using CORBA::ULong;

namespace hpp {
  namespace gui {
    using gepetto::gui::MainWindow;
    typedef graphics::WindowsManager::Color_t OsgColor_t;
    typedef graphics::Configuration OsgConfiguration_t;
    typedef gepetto::gui::ActionSearchBar ActionSearchBar;

    HppWidgetsPlugin::JointElement::JointElement (
        const std::string& n, const std::string& prefix,
        const hpp::Names_t& bns, JointTreeItem* i, bool updateV)
      : name (n), prefix (prefix),
      bodyNames (bns.length()), item (i), updateViewer (bns.length(), updateV)
    {
      for (std::size_t i = 0; i < bns.length(); ++i)
        bodyNames[i] = std::string(bns[(CORBA::ULong)i]);
    }

    HppWidgetsPlugin::HppWidgetsPlugin() :
      pathPlayer_ (NULL),
      solverWidget_ (NULL),
      configListWidget_ (NULL),
      hpp_ (NULL),
      jointTreeWidget_ (NULL)
    {
    }

    HppWidgetsPlugin::~HppWidgetsPlugin()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      foreach (QDockWidget* dock, dockWidgets_) {
        main->removeDockWidget(dock);
        delete dock;
      }
      closeConnection ();
    }

    void HppWidgetsPlugin::init()
    {
      openConnection();

      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      QDockWidget* dock;

      // Configuration list widget
      dock = new QDockWidget ("&Configuration List", main);
      configListWidget_ = new ConfigurationListWidget (this, dock);
      dock->setWidget(configListWidget_);
      main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_C);
      dockWidgets_.append(dock);
      main->registerShortcut("Configuration List", "Toggle view", dock->toggleViewAction());

      // Solver widget
      dock = new QDockWidget ("Problem &solver", main);
      solverWidget_ = new SolverWidget (this, dock);
      dock->setWidget(solverWidget_);
      main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Horizontal);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_S);
      dockWidgets_.append(dock);
      main->registerShortcut("Problem solver", "Toggle view", dock->toggleViewAction());

      // Path player widget
      dock = new QDockWidget ("&Path player", main);
      pathPlayer_ = new PathPlayer (this, dock);
      dock->setWidget(pathPlayer_);
      main->insertDockWidget (dock, Qt::BottomDockWidgetArea, Qt::Horizontal);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_P);
      dockWidgets_.append(dock);
      main->registerShortcut("PathPlayer", "Toggle view", dock->toggleViewAction());

      // Joint tree widget
      dock = new QDockWidget ("&Joint Tree", main);
      jointTreeWidget_ = new JointTreeWidget (this, dock);
      dock->setWidget(jointTreeWidget_);
      jointTreeWidget_->dockWidget (dock);
      main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_J);
      dockWidgets_.append(dock);
      main->registerShortcut("JointTree", "Toggle view", dock->toggleViewAction());

      loadConstraintWidget();

      // Connect widgets
      connect (solverWidget_, SIGNAL (problemSolved ()), pathPlayer_, SLOT (update()));

      connect (main, SIGNAL (refresh()), SLOT (update()));

      connect (main, SIGNAL (configurationValidation ()),
          SLOT (configurationValidation ()));
      main->connect (this, SIGNAL (configurationValidationStatus (bool)),
          SLOT (configurationValidationStatusChanged (bool)));
      main->connect (this, SIGNAL (configurationValidationStatus (QStringList)),
          SLOT (configurationValidationStatusChanged (QStringList)));
      connect (main, SIGNAL (applyCurrentConfiguration()),
          SLOT (applyCurrentConfiguration()));
      connect (main, SIGNAL (selectJointFromBodyName (QString)),
          SLOT (selectJointFromBodyName (QString)), Qt::QueuedConnection);
      main->connect (this, SIGNAL (logJobFailed(int,QString)),
          SLOT (logJobFailed(int, QString)));
      main->connect (this, SIGNAL (logSuccess(QString)), SLOT (log(QString)));
      main->connect (this, SIGNAL (logFailure(QString)), SLOT (logError(QString)));

      main->osg()->createGroup("joints");
      main->osg()->addToGroup("joints", "hpp-gui");
      main->osg()->refresh();

      main->registerSlot("requestCreateJointGroup", this);
      main->registerSlot("requestCreateComGroup", this);
      main->registerSlot("setRobotVelocity", pathPlayer_);
      main->registerSlot("lengthBetweenRefresh", pathPlayer_);
      main->registerSlot("getCurrentPath", pathPlayer_);
      main->registerSlot("getHppIIOPurl", this);
      main->registerSlot("getSelectedJoint", jointTreeWidget_);
      main->registerSignal(SIGNAL(appliedConfigAtParam(int,double)), pathPlayer_);

      ActionSearchBar* asb = main->actionSearchBar();
      JointAction* a;

      a = new JointAction (tr("Add joint &frame"), jointTreeWidget_, this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (addJointFrame(std::string)));
      asb->addAction(a);

      a = new JointAction (tr("Display &roadmap"), jointTreeWidget_, this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (displayRoadmap(std::string)));
      asb->addAction(a);
    }

    QString HppWidgetsPlugin::name() const
    {
      return QString ("Widgets for hpp-corbaserver");
    }

    void HppWidgetsPlugin::loadRobotModel(gepetto::gui::DialogLoadRobot::RobotDefinition rd)
    {
      client()->robot()->loadRobotModel(
          gepetto::gui::Traits<QString>::to_corba(rd.robotName_    ).in(),
          gepetto::gui::Traits<QString>::to_corba(rd.rootJointType_).in(),
          gepetto::gui::Traits<QString>::to_corba(rd.package_      ).in(),
          gepetto::gui::Traits<QString>::to_corba(rd.modelName_    ).in(),
          gepetto::gui::Traits<QString>::to_corba(rd.urdfSuf_      ).in(),
          gepetto::gui::Traits<QString>::to_corba(rd.srdfSuf_      ).in());
      // This is already done in requestRefresh
      // jointTreeWidget_->reload();
      gepetto::gui::MainWindow::instance()->requestRefresh();
      gepetto::gui::MainWindow::instance()->requestApplyCurrentConfiguration();
      emit logSuccess ("Robot " + rd.name_ + " loaded");
    }

    void HppWidgetsPlugin::loadEnvironmentModel(gepetto::gui::DialogLoadEnvironment::EnvironmentDefinition ed)
    {
      QString prefix = ed.envName_ + "/";
      client()->obstacle()->loadObstacleModel(
          gepetto::gui::Traits<QString>::to_corba(ed.package_     ).in(),
          gepetto::gui::Traits<QString>::to_corba(ed.urdfFilename_).in(),
          gepetto::gui::Traits<QString>::to_corba(prefix          ).in());
      computeObjectPosition ();
      gepetto::gui::MainWindow::instance()->requestRefresh();
      emit logSuccess ("Environment " + ed.name_ + " loaded");
    }

    std::string HppWidgetsPlugin::getBodyFromJoint(const std::string &jointName) const
    {
      JointMap::const_iterator itj = jointMap_.find(jointName);
      if (itj == jointMap_.constEnd()) return std::string();
      return itj->prefix + itj->bodyNames[0];
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

    QString HppWidgetsPlugin::getHppIIOPurl () const
    {
      QString host = gepetto::gui::MainWindow::instance ()->settings_->getSetting
        ("hpp/host", QString ()).toString ();
      QString port = gepetto::gui::MainWindow::instance ()->settings_->getSetting
        ("hpp/port", QString ()).toString ();
      return gepetto::gui::omniOrb::IIOPurl (host, port);
    }

    void HppWidgetsPlugin::openConnection ()
    {
      closeConnection ();
      hpp_ = new hpp::corbaServer::Client (0,0);
      QByteArray iiop = getHppIIOPurl ().toAscii();
      hpp_->connect (iiop.constData ());
    }

    void HppWidgetsPlugin::closeConnection ()
    {
      if (hpp_) delete hpp_;
      hpp_ = NULL;
    }

    void HppWidgetsPlugin::applyCurrentConfiguration()
    {
      gepetto::gui::MainWindow * main = gepetto::gui::MainWindow::instance ();
      if (jointMap_.isEmpty()) {
          if (QMessageBox::Ok == QMessageBox::question (NULL, "Refresh required",
                                 "The current configuration cannot be applied because the joint map is empty. "
                                 "This is probably because you are using external commands (python "
                                 "interface) and you did not refresh this GUI. "
                                 "Do you want to refresh the joint map now ?"))
            jointTreeWidget_->reload();
          else
            emit logFailure("The current configuration cannot be applied. "
                            "This is probably because you are using external commands (python "
                            "interface) and you did not refresh this GUI. "
                            "Use the refresh button \"Tools\" menu.");
      }
      // Something smarter could be done here.
      // For instance, the joint tree item could know the NodePtr_t of their bodies.
      OsgConfiguration_t T;
      for (JointMap::iterator ite = jointMap_.begin ();
          ite != jointMap_.end (); ite++) {
        for (std::size_t i = 0; i < ite->bodyNames.size(); ++i)
        {
          hpp::Transform__var t = client()->robot()->getLinkPosition(ite->bodyNames[i].c_str());
          fromHPP(t, T);
          if (ite->updateViewer[i]) {
            std::string n = ite->prefix + ite->bodyNames[i];
            ite->updateViewer[i] = main->osg()->applyConfiguration(n, T);
          }
        }
        if (!ite->item) continue;
        if (ite->item->config().length() > 0) {
          hpp::floatSeq_var c = client()->robot ()->getJointConfig (ite->name.c_str());
          ite->item->updateConfig (c.in());
        }
      }
      for (std::list<std::string>::const_iterator it = jointFrames_.begin ();
          it != jointFrames_.end (); ++it) {
        std::string n = escapeJointName(*it);
        hpp::Transform__var t = client()->robot()->getJointPosition(it->c_str());
        fromHPP(t, T);
        main->osg()->applyConfiguration (n, T);
      }
      T.quat.set(0,0,0,1);
      for (std::list<std::string>::const_iterator it = comFrames_.begin ();
          it != comFrames_.end (); ++it) {
        std::string n = "com_" + escapeJointName(*it);
        hpp::floatSeq_var t = client()->robot()->getPartialCom(it->c_str());
        fromHPP (t, T.position);
        main->osg()->applyConfiguration (n, T);
      }
      main->osg()->refresh();
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
        emit logFailure(QString (e.msg));
        return;
      }
      static QRegExp collision ("Collision between object (.*) and (.*)");
      QStringList col;
      if (!bb) {
        if (collision.exactMatch(QString::fromLocal8Bit(report))) {
          CORBA::String_var robotName = client ()->robot()->getRobotName();
          size_t pos = strlen(robotName) + 1;
          for (int i = 1; i < 3; ++i) {
            std::string c = collision.cap (i).toStdString();
            bool found = false;
            foreach (const JointElement& je, jointMap_) {
              for (std::size_t j = 0; j < je.bodyNames.size(); ++j) {
                if (je.bodyNames[j].length() <= pos)
                  continue;
                size_t len = je.bodyNames[j].length() - pos;
                if (je.bodyNames[j].compare(pos, len, c, 0, len) == 0) {
                  col.append(QString::fromStdString(je.bodyNames[j]));
                  found = true;
                  break;
                }
              }
              if (found) break;
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

    void HppWidgetsPlugin::selectJointFromBodyName(const QString bodyName)
    {
      boost::regex roadmap ("^(roadmap|path[0-9]+)_(.*)/(node|edge)([0-9]+)$");
      boost::cmatch what;
      const std::string bname = bodyName.toStdString();
      if (boost::regex_match (bname.c_str(), what, roadmap)) {
        std::string group; group.assign(what[1].first, what[1].second);
        std::string joint; joint.assign(what[2].first, what[2].second);
        std::string type;  type .assign(what[3].first, what[3].second);
        CORBA::ULong n = (CORBA::ULong)std::atoi (what[4].first);
        qDebug () << "Detected the" << group.c_str() << type.c_str() << n << "of joint" << joint.c_str();
        if (group == "roadmap") {
          if (type == "node") {
            try {
              hpp::floatSeq_var q = hpp_->problem()->node(n);
              hpp_->robot()->setCurrentConfig(q.in());
              gepetto::gui::MainWindow::instance()->requestApplyCurrentConfiguration();
            } catch (const hpp::Error& e) {
              emit logFailure(QString::fromLocal8Bit(e.msg));
            }
          } else if (type == "edge") {
            // TODO
          }
        } else if (group[0] == 'p') {
          if (type == "node") {
            int pid = std::atoi(&group[4]);
            // compute pid
            hpp::floatSeq_var times;
            hpp::floatSeqSeq_var waypoints = hpp_->problem()->getWaypoints((CORBA::UShort)pid, times.out());
            if (n < waypoints->length()) {
              hpp_->robot()->setCurrentConfig(waypoints[n]);
              MainWindow::instance()->requestApplyCurrentConfiguration();
            }
          }
        }
        return;
      }
      foreach (const JointElement& je, jointMap_) {
        // je.bodyNames will be of size 1 most of the time
        // so it is fine to use a vector + line search, vs map + binary
        // FIXME A good intermediate is to sort the vector.
        const std::size_t len = je.prefix.length();
        if (bname.compare(0, len, je.prefix) == 0) {
          for (std::size_t i = 0; i < je.bodyNames.size(); ++i) {
            if (bname.compare(len, std::string::npos, je.bodyNames[i]) == 0) {
              // TODO: use je.item for a faster selection.
              jointTreeWidget_->selectJoint (je.name);
              return;
            }
          }
        }
      }
      qDebug () << "Joint for body" << bodyName << "not found.";
    }

    void HppWidgetsPlugin::update()
    {
      jointTreeWidget_->reload();
      pathPlayer_->update();
      solverWidget_->update();
      configListWidget_->fetchInitAndGoalConfigs();
      constraintWidget_->reload();
    }

    QString HppWidgetsPlugin::requestCreateJointGroup(const QString jn)
    {
      return createJointGroup(jn.toStdString()).c_str();
    }

    QString HppWidgetsPlugin::requestCreateComGroup(const QString com)
    {
      return QString::fromStdString(createComGroup(com.toStdString()));
    }

    HppWidgetsPlugin::HppClient *HppWidgetsPlugin::client() const
    {
      return hpp_;
    }

    HppWidgetsPlugin::JointMap &HppWidgetsPlugin::jointMap()
    {
      return jointMap_;
    }

    PathPlayer* HppWidgetsPlugin::pathPlayer() const
    {
      return pathPlayer_;
    }

    JointTreeWidget* HppWidgetsPlugin::jointTreeWidget() const
    {
      return jointTreeWidget_;
    }

    void HppWidgetsPlugin::updateRobotJoints(const QString robotName)
    {
      hpp::Names_t_var joints = client()->robot()->getAllJointNames ();
      jointMap_.clear();
      for (size_t i = 0; i < joints->length (); ++i) {
        const char* jname = joints[(ULong) i];
        hpp::Names_t_var lnames = client()->robot()->getLinkNames (jname);
        std::string prefix (robotName.toStdString() + "/");
        jointMap_[jname] = JointElement(jname, prefix, lnames, 0, true);
      }
    }

    std::string HppWidgetsPlugin::getSelectedJoint() const
    {
      return jointTreeWidget_->selectedJoint();
    }

    Roadmap* HppWidgetsPlugin::createRoadmap(const std::string &jointName)
    {
      Roadmap* r = new Roadmap (this);
      r->initRoadmapFromJoint(jointName);
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
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      std::string target = createJointGroup(jointName);
      const std::string n = target + "/XYZ";
      const OsgColor_t color(1,0,0,1);

      /// This returns false if the frame already exists
      if (main->osg()->addXYZaxis (n, color, 0.005f, 0.015f)) {
        main->osg()->setVisibility (n, "ALWAYS_ON_TOP");
        return;
      } else {
        main->osg()->setVisibility (n, "ALWAYS_ON_TOP");
      }
    }

    void HppWidgetsPlugin::computeObjectPosition()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      hpp::Names_t_var obs = client()->obstacle()->getObstacleNames (true, false);
      hpp::Transform__var cfg = hpp::Transform__alloc () ;
      OsgConfiguration_t d;
      for (ULong i = 0; i < obs->length(); ++i) {
        client()->obstacle()->getObstaclePosition (obs[i], cfg.out());
        fromHPP(cfg, d);
        main->osg ()->applyConfiguration(std::string(obs[i]), d);
      }
      main->osg()->refresh();
    }

    void HppWidgetsPlugin::loadConstraintWidget()
    {
      MainWindow* main = MainWindow::instance();
      QDockWidget* dock = new QDockWidget ("&Constraint creator", main);
      constraintWidget_ = new ConstraintWidget (this, dock);
      dock->setWidget(constraintWidget_);
      main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_V);
      dockWidgets_.append(dock);
      constraintWidget_->addConstraint(new PositionConstraint(this));
      constraintWidget_->addConstraint(new OrientationConstraint(this));
      constraintWidget_->addConstraint(new TransformConstraint(this));
      constraintWidget_->addConstraint(new LockedJointConstraint(this));
      main->registerShortcut("Constraint Widget", "Toggle view", dock->toggleViewAction());
    }

    std::string HppWidgetsPlugin::escapeJointName(const std::string jn)
    {
      std::string target = jn;
      boost::replace_all (target, "/", "__");
      return target;
    }

    std::string HppWidgetsPlugin::createJointGroup(const std::string jn)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      std::string target = escapeJointName(jn);
      graphics::GroupNodePtr_t group = main->osg()->getGroup (target.c_str(), false);
      if (group) return target;
      if (!main->osg()->getGroup(target)) {
        main->osg()->createGroup(target);
        main->osg()->addToGroup(target, "joints");

        hpp::Transform__var t = client()->robot()->getJointPosition (jn.c_str());
        OsgConfiguration_t p;
        fromHPP(t, p);
        jointFrames_.push_back(jn);
        main->osg()->applyConfiguration (target, p);
        main->osg()->refresh();
      }
      return target;
    }

    std::string HppWidgetsPlugin::createComGroup(const std::string com)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      std::string target = "com_" + escapeJointName(com);
      graphics::GroupNodePtr_t group = main->osg()->getGroup (target.c_str(), false);
      if (group) return target;
      if (!main->osg()->getGroup(target)) {
        main->osg()->createGroup(target);
        main->osg()->addToGroup(target, "joints");

        hpp::floatSeq_var p = client()->robot()->getPartialCom (com.c_str());
        OsgConfiguration_t t;
        t.quat.set(0,0,0,1);
        fromHPP (p, t.position);
        comFrames_.push_back(com);
        main->osg()->applyConfiguration (target, t);
        main->osg()->refresh();
      }
      return target;
    }

    Q_EXPORT_PLUGIN2 (hppwidgetsplugin, HppWidgetsPlugin)
  } // namespace gui
} // namespace hpp
