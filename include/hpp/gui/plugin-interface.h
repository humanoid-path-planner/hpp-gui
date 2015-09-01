#ifndef PLUGININTERFACE_HH
#define PLUGININTERFACE_HH

#include <QtGui>
#include <QWidget>

#include <hpp/gui/dialog/dialogloadrobot.h>
#include <hpp/gui/dialog/dialogloadenvironment.h>

#include <omniORB4/CORBA.h>

class PluginInterface {
public:
  virtual ~PluginInterface () {}

  virtual QString name () const = 0;

  void doInit ()
  {
    try {
      init ();
    } catch (const std::exception& e) {
      errorMsg_ = QString (e.what ());
    }
  };

  bool isInit () const
  {
    return errorMsg_.isNull ();
  }

  const QString& errorMsg () const
  {
    return errorMsg_;
  }

protected:
  virtual void init () = 0;

private:
  QString errorMsg_;
};

Q_DECLARE_INTERFACE (PluginInterface, "hpp-gui.plugins/0.0")


class JointAction : public QAction
{
  Q_OBJECT

public:
  JointAction (const QString& actionName, const std::string& jointName, QObject* parent)
    : QAction (actionName, parent)
    , jointName_ (jointName)
  {
    connect (this, SIGNAL (triggered(bool)), SLOT(trigger()));
  }

signals:
  void triggered (const std::string jointName);

private slots:
  void trigger () {
    emit triggered(jointName_);
  }

private:
  const std::string jointName_;
};

class JointModifierInterface {
public:
  virtual ~JointModifierInterface () {}

  virtual JointAction* action (const std::string& jointName) const = 0;
};

Q_DECLARE_INTERFACE (JointModifierInterface, "hpp-gui.plugin.joint-modifier/0.0")

class ModelInterface {
public:
  virtual ~ModelInterface () {}

  virtual void loadRobotModel (DialogLoadRobot::RobotDefinition rd) = 0;

  virtual void loadEnvironmentModel (DialogLoadEnvironment::EnvironmentDefinition ed) = 0;

  virtual std::string getBodyFromJoint (const std::string& jointName) const = 0;
};

Q_DECLARE_INTERFACE (ModelInterface, "hpp-gui.plugin.model/0.0")

class CorbaErrorInterface {
public:
  virtual ~CorbaErrorInterface () {}

  /// return true if error was handled.
  virtual bool corbaException (int jobId, const CORBA::Exception& excep) const = 0;
};

Q_DECLARE_INTERFACE (CorbaErrorInterface, "hpp-gui.plugin.corbaerror/0.0")
#endif // PLUGININTERFACE_HH
