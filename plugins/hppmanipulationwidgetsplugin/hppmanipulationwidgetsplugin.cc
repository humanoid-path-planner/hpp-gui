#include "hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh"

#include "hppmanipulationwidgetsplugin/roadmap.hh"
#include "hpp/gui/mainwindow.hh"
#include "hpp/gui/windows-manager.hh"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

using CORBA::ULong;

namespace hpp {
  namespace gui {
    HppManipulationWidgetsPlugin::HppManipulationWidgetsPlugin() :
      HppWidgetsPlugin (),
      hpp_ (NULL),
      toolBar_ (NULL)
    {
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
    }

    QString HppManipulationWidgetsPlugin::name() const
    {
      return QString ("Widgets for hpp-manipulation-corba");
    }

    void HppManipulationWidgetsPlugin::loadRobotModel(DialogLoadRobot::RobotDefinition rd)
    {
      /// TODO: load the robot properly
      HppWidgetsPlugin::loadRobotModel (rd);
    }

    void HppManipulationWidgetsPlugin::loadEnvironmentModel(DialogLoadEnvironment::EnvironmentDefinition ed)
    {
      /// TODO: load the environment properly
      HppWidgetsPlugin::loadEnvironmentModel (ed);
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
      hpp_->connect ();
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

    Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
  } // namespace gui
} // namespace hpp
