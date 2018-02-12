#ifndef HPP_GUI_HPPCORBASERVERPLUGIN_HH
#define HPP_GUI_HPPCORBASERVERPLUGIN_HH

#include <gepetto/gui/plugin-interface.hh>
#include <gepetto/gui/omniorb/omniorbthread.hh>


/// namespace that encapsulate all the softwares of humanoid-path-planner
namespace hpp {
  /// namespace that encapsulate the hpp's plugin for gepetto-gui
  namespace gui {
    /// HppCorbaserverPlugin allows to launch a corbaserver when gui is launch
    class HppCorbaserverPlugin : public QObject,
    public gepetto::gui::PluginInterface
    {
      Q_OBJECT
      Q_INTERFACES (gepetto::gui::PluginInterface)
#ifndef USE_QT4
      Q_PLUGIN_METADATA (IID "hpp-gui.hppcorbaserverplugin")
#endif // USE_QT4

      public:
        explicit HppCorbaserverPlugin ();

        virtual ~HppCorbaserverPlugin ();

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
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPCORBASERVERPLUGIN_HH
