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

void HppManipulationWidgetsPlugin::selectJointFromBodyName(const std::string &bodyName)
{
  /// TODO: fix this
  HppWidgetsPlugin::selectJointFromBodyName (bodyName);
}

HppManipulationWidgetsPlugin::HppManipClient *HppManipulationWidgetsPlugin::manipClient() const
{
  return hpp_;
}

void HppManipulationWidgetsPlugin::updateRobotJoints(const QString robotName)
{
  /// TODO: fix this
  HppWidgetsPlugin::updateRobotJoints (robotName);
}

void HppManipulationWidgetsPlugin::displayRoadmap (const std::string& jointName)
{
  ManipulationRoadmap r (this);
  r.initRoadmap (jointName);
  r.displayRoadmap ();
}

Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
