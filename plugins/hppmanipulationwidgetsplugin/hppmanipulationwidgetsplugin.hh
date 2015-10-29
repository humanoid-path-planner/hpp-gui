#ifndef HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH
#define HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH

#include <QToolBar>

#include <hpp/gui/plugin-interface.hh>
#include <hpp/corbaserver/manipulation/client.hh>
#undef __robot_hh__
#undef __problem_hh__
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace hpp {
  namespace gui {
    class HppManipulationWidgetsPlugin : public HppWidgetsPlugin
                                         // , public PluginInterface, public ModelInterface, public CorbaErrorInterface
    {
      Q_OBJECT
        Q_INTERFACES (hpp::gui::PluginInterface
            hpp::gui::ModelInterface
            hpp::gui::CorbaInterface)

      public:
        typedef hpp::corbaServer::manipulation::Client HppManipClient;

        explicit HppManipulationWidgetsPlugin ();

        virtual ~HppManipulationWidgetsPlugin ();

        // PluginInterface interface
      public:
        void init();
        QString name() const;

        // ModelInterface interface
      public:
        void loadRobotModel (DialogLoadRobot::RobotDefinition rd);
        void loadEnvironmentModel (DialogLoadEnvironment::EnvironmentDefinition ed);
        std::string getBodyFromJoint (const std::string& jointName) const;
signals:
        void configurationValidationStatus (bool valid);

        // CorbaInterface
      public:
        virtual void openConnection ();
        virtual void closeConnection();

      public:
        HppManipClient* manipClient () const;

        void updateRobotJoints (const QString robotName);

        virtual Roadmap* createRoadmap (const std::string& jointName);

        public slots:
          void drawRobotContacts ();
          void drawEnvironmentContacts ();

      private:
        HppManipClient* hpp_;

        QToolBar *toolBar_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH
