#ifndef PLUGININTERFACE_HH
#define PLUGININTERFACE_HH

#include <QtGui>
#include <QWidget>

class PluginInterface {
public:
  virtual ~PluginInterface () {}

  virtual void init () = 0;
};

Q_DECLARE_INTERFACE (PluginInterface, "hpp-gui.plugins/0.0")

class AttitudeDeviceInterface : public PluginInterface {
public:
  virtual ~AttitudeDeviceInterface () {}

  virtual void newDevice (const std::string& jointName) = 0;
};

Q_DECLARE_INTERFACE (AttitudeDeviceInterface, "hpp-gui.plugin.attitude-device/0.0")

#endif // PLUGININTERFACE_HH
