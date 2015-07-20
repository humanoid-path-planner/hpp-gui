#ifndef HPPWIDGETSPLUGIN_HH
#define HPPWIDGETSPLUGIN_HH

#include <hpp/gui/plugin-interface.h>
#include <hpp/corbaserver/client.hh>

class SolverWidget;
class PathPlayer;

class HppWidgetsPlugin : public QObject, public PluginInterface
{
  Q_OBJECT
  Q_INTERFACES (PluginInterface)

  typedef hpp::corbaServer::Client HppClient;

public:
  explicit HppWidgetsPlugin ();

  virtual ~HppWidgetsPlugin ();

signals:

public slots:

  // PluginInterface interface
public:
  void init();
  QString name() const;

  HppClient* client () const;

  PathPlayer* pathPlayer_;
  SolverWidget* solverWidget_;

private:
  HppClient* hpp_;
};

#endif // HPPWIDGETSPLUGIN_HH
