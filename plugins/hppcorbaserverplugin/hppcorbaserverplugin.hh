#ifndef HPP_GUI_HPPCORBASERVERPLUGIN_HH
#define HPP_GUI_HPPCORBASERVERPLUGIN_HH

#include <gepetto/gui/plugin-interface.hh>
#include <gepetto/gui/omniorb/omniorbthread.hh>

namespace hpp {
  namespace gui {
    class HppCorbaserverPlugin : public QObject,
    public gepetto::gui::PluginInterface
    {
      Q_OBJECT
      Q_INTERFACES (gepetto::gui::PluginInterface)

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
          gepetto::gui::CorbaServer* server_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPCORBASERVERPLUGIN_HH
