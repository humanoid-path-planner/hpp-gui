#include "hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh"

#include "hppmanipulationwidgetsplugin/roadmap.hh"

#define QSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.toStdString().c_str())
#define STDSTRING_TO_CONSTCHARARRAY(qs) ((const char*)qs.c_str())

using CORBA::ULong;

HppManipulationWidgetsPlugin::HppManipulationWidgetsPlugin() :
  HppWidgetsPlugin (),
  hpp_ (new HppManipClient (0,0))
{
  hpp_->connect ();
}

HppManipulationWidgetsPlugin::~HppManipulationWidgetsPlugin()
{
  if (hpp_)
    delete hpp_;
}

void HppManipulationWidgetsPlugin::init()
{
  HppWidgetsPlugin::init ();
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

Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
