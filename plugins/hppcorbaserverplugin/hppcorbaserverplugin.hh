#ifndef HPP_GUI_HPPCORBASERVERPLUGIN_HH
#define HPP_GUI_HPPCORBASERVERPLUGIN_HH

#include <hpp/gui/plugin-interface.hh>
#include <hpp/gui/omniorb/omniorbthread.hh>

namespace hpp {
  namespace gui {
    class HppCorbaserverPlugin : public QObject,
    public PluginInterface
    {
      Q_OBJECT
        Q_INTERFACES (hpp::gui::PluginInterface)

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
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPCORBASERVERPLUGIN_HH
