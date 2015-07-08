#include <QObject>
#include <QWidget>

#include <iostream>

#include <hpp/gui/plugin-interface.h>

class TestPlugin : public QObject, public PluginInterface {
  Q_OBJECT
  Q_INTERFACES (PluginInterface)

public:
  void init();
  QWidget* widget ();
};
