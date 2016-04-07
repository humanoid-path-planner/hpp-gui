#include "hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh"

#include "hppmanipulationwidgetsplugin/roadmap.hh"
#include "hpp/gui/mainwindow.hh"
#include "hpp/gui/windows-manager.hh"

#include "hppwidgetsplugin/jointtreewidget.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
    HppManipulationWidgetsPlugin::HppManipulationWidgetsPlugin() :
      HppWidgetsPlugin (),
      hpp_ (NULL),
      toolBar_ (NULL)
    {
      firstEnter_ = 0;
    }

    HppManipulationWidgetsPlugin::~HppManipulationWidgetsPlugin()
    {
    }

    void HppManipulationWidgetsPlugin::init()
    {
      HppWidgetsPlugin::init ();

      toolBar_ = MainWindow::instance()->addToolBar("Manipulation tools");
      QAction* drawRContact = new QAction ("Draw robot contacts",toolBar_);
      QAction* drawEContact = new QAction ("Draw environment contacts",toolBar_);
      toolBar_->addAction (drawRContact);
      toolBar_->addAction (drawEContact);
      connect (drawRContact, SIGNAL(triggered()), SLOT (drawRobotContacts()));
      connect (drawEContact, SIGNAL(triggered()), SLOT (drawEnvironmentContacts()));
      QAction* drawHFrame = new QAction ("Draw handles frame",toolBar_);
      QAction* drawGFrame = new QAction ("Draw grippers frame",toolBar_);
      toolBar_->addAction (drawHFrame);
      toolBar_->addAction (drawGFrame);
      connect (drawHFrame, SIGNAL(triggered()), SLOT (drawHandlesFrame()));
      connect (drawGFrame, SIGNAL(triggered()), SLOT (drawGrippersFrame()));
    }

    QString HppManipulationWidgetsPlugin::name() const
    {
      return QString ("Widgets for hpp-manipulation-corba");
    }

    void HppManipulationWidgetsPlugin::loadRobotModel(DialogLoadRobot::RobotDefinition rd)
    {
      if (firstEnter_ == 0) {
      	hpp_->robot ()->create (Traits<QString>::to_corba("composite").in());
      	firstEnter_ = 1;
      }
      hpp_->robot ()->insertRobotModel (Traits<QString>::to_corba(rd.robotName_).in(),
					Traits<QString>::to_corba(rd.rootJointType_).in(),
					Traits<QString>::to_corba(rd.package_).in(),
					Traits<QString>::to_corba(rd.modelName_).in(),
					Traits<QString>::to_corba(rd.urdfSuf_).in(),
					Traits<QString>::to_corba(rd.srdfSuf_).in());
      updateRobotJoints (rd.robotName_);
      jointTreeWidget_->addJointToTree("base_joint", 0);
      applyCurrentConfiguration();
      emit logSuccess ("Robot " + rd.name_ + " loaded");
    }

    void HppManipulationWidgetsPlugin::loadEnvironmentModel(DialogLoadEnvironment::EnvironmentDefinition ed)
    {
      if (firstEnter_ == 0) {
      	hpp_->robot ()->create (Traits<QString>::to_corba("composite").in());
      	firstEnter_ = 1;
      }
      hpp_->robot ()-> loadEnvironmentModel(Traits<QString>::to_corba(ed.package_).in(),
					    Traits<QString>::to_corba(ed.urdfFilename_).in(),
					    Traits<QString>::to_corba(ed.urdfSuf_).in(),
					    Traits<QString>::to_corba(ed.srdfSuf_).in(),
					    Traits<QString>::to_corba(ed.name_ + "/").in());
      HppWidgetsPlugin::computeObjectPosition();
      emit logSuccess ("Environment " + ed.name_ + " loaded");
   }

    std::string HppManipulationWidgetsPlugin::getBodyFromJoint(const std::string &jointName) const
    {
      /// TODO: fix this
      return HppWidgetsPlugin::getBodyFromJoint (jointName);
    }

    void HppManipulationWidgetsPlugin::openConnection()
    {
      HppWidgetsPlugin::openConnection();
      hpp_ = new HppManipClient (0,0);
      QByteArray iiop = getIIOPurl ().toAscii();
      hpp_->connect (iiop.constData ());
    }

    void HppManipulationWidgetsPlugin::closeConnection()
    {
      HppWidgetsPlugin::closeConnection();
      if (hpp_) delete hpp_;
      hpp_ = NULL;
    }

    HppManipulationWidgetsPlugin::HppManipClient *HppManipulationWidgetsPlugin::manipClient() const
    {
      return hpp_;
    }

    void HppManipulationWidgetsPlugin::updateRobotJoints(const QString robotName)
    {
      Q_UNUSED (robotName);
      hpp::Names_t_var joints = client()->robot()->getAllJointNames ();
      std::cout << joints->length() << std::endl;
      for (size_t i = 0; i < joints->length (); ++i) {
        const char* jname = joints[(ULong) i];
        std::string linkName (client()->robot()->getLinkName (jname));
        jointMap_[jname] = JointElement(jname, linkName, 0, true);
      }
    }

    Roadmap *HppManipulationWidgetsPlugin::createRoadmap(const std::string &jointName)
    {
      ManipulationRoadmap* r = new ManipulationRoadmap(this);
      r->initRoadmap(jointName);
      return r;
    }

    void HppManipulationWidgetsPlugin::drawRobotContacts()
    {
      MainWindow* main = MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getRobotContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getRobotContact (
            rcs[i], indexes.out(), points.out());
        CORBA::Long iPts = 0;
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Create group
          std::string target = createJointGroup(std::string(cjs[j]));

          /// Add the contacts
          std::stringstream ssname;
          ssname <<  target << "/contact_"
            << escapeJointName(std::string (rcs[i])) << "_" << j;
          std::string name = ssname.str ();
          gepetto::corbaserver::PositionSeq ps; ps.length (indexes[j] - iPts);
          for (CORBA::Long k = iPts; k < indexes[j]; ++k) {
            ps[k - iPts][0] = (float)points[k][0];
            ps[k - iPts][1] = (float)points[k][1];
            ps[k - iPts][2] = (float)points[k][2];
          }
          iPts = indexes[j];
          main->osg()->addCurve (name.c_str(), ps, color);
          main->osg()->setCurveMode (name.c_str(), GL_POLYGON);
        }
      }
    }

    void HppManipulationWidgetsPlugin::drawEnvironmentContacts()
    {
      MainWindow* main = MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getEnvironmentContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getEnvironmentContact (
            rcs[i], indexes.out(), points.out());
        CORBA::Long iPts = 0;
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Add the contacts
          std::stringstream ssname;
          ssname << "hpp-gui/contact_"
            << escapeJointName(std::string (rcs[i])) << "_" << j;
          std::string name = ssname.str ();
          gepetto::corbaserver::PositionSeq ps; ps.length (indexes[j] - iPts);
          for (CORBA::Long k = iPts; k < indexes[j]; ++k) {
            ps[k - iPts][0] = (float)points[k][0];
            ps[k - iPts][1] = (float)points[k][1];
            ps[k - iPts][2] = (float)points[k][2];
          }
          iPts = indexes[j];
          main->osg()->addCurve (name.c_str(), ps, color);
          main->osg()->setCurveMode (name.c_str(), GL_POLYGON);
        }
      }
    }

    void HppManipulationWidgetsPlugin::drawHandlesFrame()
    {
      MainWindow* main = MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getAvailable("handle");
      hpp::Transform__var t (new Transform_);
      graphics::WindowsManager::value_type t_gv[7];
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        const std::string jn =
          hpp_->robot()->getHandlePositionInJoint (rcs[i],t.out());
        std::string groupName = createJointGroup (jn.c_str());
        std::string hn = "handle_" + escapeJointName (std::string(rcs[i]));
        for (int i = 0; i < 7; ++i) t_gv[i] = t.in()[i];
        main->osg()->addXYZaxis (hn.c_str(), color, 0.005f, 1.f);
        main->osg()->applyConfiguration (hn.c_str(), t_gv);
        main->osg()->addToGroup (hn.c_str(), groupName.c_str());
      }
      main->osg()->refresh();
    }

    void HppManipulationWidgetsPlugin::drawGrippersFrame()
    {
      MainWindow* main = MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getAvailable("gripper");
      hpp::Transform__var t (new Transform_);
      graphics::WindowsManager::value_type t_gv[7];
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        const std::string jn =
          hpp_->robot()->getGripperPositionInJoint (rcs[i],t.out());
        std::string groupName = createJointGroup (jn.c_str());
        std::string hn = "gripper_" + escapeJointName (std::string(rcs[i]));
        for (int i = 0; i < 7; ++i) t_gv[i] = t.in()[i];
        main->osg()->addXYZaxis (hn.c_str(), color, 0.005f, 1.f);
        main->osg()->applyConfiguration (hn.c_str(), t_gv);
        main->osg()->addToGroup (hn.c_str(), groupName.c_str());
      }
      main->osg()->refresh();
    }

    Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
  } // namespace gui
} // namespace hpp
