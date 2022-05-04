//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_HPPCORBASERVERPLUGIN_HH
#define HPP_GUI_HPPCORBASERVERPLUGIN_HH

#include <gepetto/gui/omniorb/omniorbthread.hh>
#include <gepetto/gui/plugin-interface.hh>

/// namespace that encapsulate all the softwares of humanoid-path-planner
namespace hpp {
/// namespace that encapsulate the hpp's plugin for gepetto-gui
namespace gui {
/// HppCorbaserverPlugin allows to launch a corbaserver when gui is launch
class HppCorbaserverPlugin : public QObject,
                             public gepetto::gui::PluginInterface {
  Q_OBJECT
  Q_INTERFACES(gepetto::gui::PluginInterface)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  Q_PLUGIN_METADATA(IID "hpp-gui.hppcorbaserverplugin")
#endif

 public:
  explicit HppCorbaserverPlugin();

  virtual ~HppCorbaserverPlugin();

 signals:

 public slots:

  // PluginInterface interface
 public:
  /// Initializes the plugin
  void init();
  /// Returns the plugin's name
  /// \return name of the plugin
  QString name() const;

 private:
  gepetto::gui::CorbaServer* server_;
};
}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_HPPCORBASERVERPLUGIN_HH
