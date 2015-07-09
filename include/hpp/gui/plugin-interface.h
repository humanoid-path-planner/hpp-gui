#ifndef PLUGININTERFACE_HH
#define PLUGININTERFACE_HH

#include <QtGui>
#include <QWidget>

class PluginInterface {
public:
  virtual ~PluginInterface () {}

  virtual void init () = 0;

  virtual QString name () const = 0;
};

Q_DECLARE_INTERFACE (PluginInterface, "hpp-gui.plugins/0.0")

class JointModifierInterface {
public:
  virtual ~JointModifierInterface () {}

  virtual QAction* action (const std::string& jointName) const = 0;
};

Q_DECLARE_INTERFACE (JointModifierInterface, "hpp-gui.plugin.joint-modifier/0.0")

#endif // PLUGININTERFACE_HH
