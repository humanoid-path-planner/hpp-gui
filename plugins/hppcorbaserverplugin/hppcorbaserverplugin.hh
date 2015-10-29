#ifndef HPP_GUI_HPPCORBASERVERPLUGIN_HH
#define HPP_GUI_HPPCORBASERVERPLUGIN_HH

#include <hpp/gui/plugin-interface.hh>
#include <hpp/gui/omniorb/omniorbthread.hh>

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

#endif // HPP_GUI_HPPCORBASERVERPLUGIN_HH
