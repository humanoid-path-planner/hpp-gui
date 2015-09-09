#ifndef HPPMANIPULATIONWIDGETSPLUGIN_HH
#define HPPMANIPULATIONWIDGETSPLUGIN_HH

#include <hpp/gui/plugin-interface.h>
#include <hpp/corbaserver/manipulation/client.hh>
#undef __robot_hh__
#undef __problem_hh__
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

class HppManipulationWidgetsPlugin : public HppWidgetsPlugin
                                     // , public PluginInterface, public ModelInterface, public CorbaErrorInterface
{
  Q_OBJECT
  Q_INTERFACES (PluginInterface ModelInterface CorbaErrorInterface)

public:
  struct JointElement {
    std::string name;
    std::string bodyName;
    JointTreeItem* item;
    bool updateViewer;

    JointElement ()
      : name (), bodyName (), item (NULL), updateViewer (false) {}
    JointElement (std::string n, std::string bn, JointTreeItem* i, bool updateV = true)
      : name (n), bodyName (bn), item (i), updateViewer (updateV) {}
  };
  typedef QMap <std::string, JointElement> JointMap;
  typedef hpp::corbaServer::manipulation::Client HppManipClient;

  explicit HppManipulationWidgetsPlugin ();

  virtual ~HppManipulationWidgetsPlugin ();

  // PluginInterface interface
public:
  void init();
  QString name() const;

  // ModelInterface interface
public:
  void loadRobotModel (DialogLoadRobot::RobotDefinition rd);
  void loadEnvironmentModel (DialogLoadEnvironment::EnvironmentDefinition ed);
  std::string getBodyFromJoint (const std::string& jointName) const;
signals:
  void configurationValidationStatus (bool valid);

public slots:
  void selectJointFromBodyName (const std::string& bodyName);

public:
  HppManipClient* manipClient () const;

  void updateRobotJoints (const QString robotName);

protected:
  void displayRoadmap (const std::string& jointName);

private:
  HppManipClient* hpp_;
};

#endif // HPPMANIPULATIONWIDGETSPLUGIN_HH
