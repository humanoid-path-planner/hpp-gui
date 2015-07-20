#ifndef HPPCORBASERVERPLUGIN_HH
#define HPPCORBASERVERPLUGIN_HH

#include <hpp/gui/plugin-interface.h>
#include <hpp/gui/omniorb/omniorbthread.h>

class HppCorbaserverPlugin : public QObject, public PluginInterface
{
  Q_OBJECT
  Q_INTERFACES (PluginInterface)

public:
  explicit HppCorbaserverPlugin ();

  virtual ~HppCorbaserverPlugin ();

signals:

public slots:

  // PluginInterface interface
public:
  void init();
  QString name() const;

private:
  CorbaServer* server_;
};

#endif // HPPCORBASERVERPLUGIN_HH
