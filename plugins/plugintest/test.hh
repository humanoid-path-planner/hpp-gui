#include <QObject>
#include <QWidget>

#include <iostream>

#include <hpp/gui/plugin-interface.hh>

class TestPlugin : public QObject, public PluginInterface {
  Q_OBJECT
  Q_INTERFACES (PluginInterface)

public:
  void init();
};
