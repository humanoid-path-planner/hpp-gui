#ifndef HPP_GUI_HPPWIDGETSPLUGIN_HH
#define HPP_GUI_HPPWIDGETSPLUGIN_HH

#include <hpp/gui/plugin-interface.hh>
#include <hpp/corbaserver/client.hh>

namespace hpp {
  namespace gui {
    class SolverWidget;
    class PathPlayer;
    class JointTreeWidget;
    class ConfigurationListWidget;
    class JointTreeItem;
    class Roadmap;

    class HppWidgetsPlugin : public QObject, public PluginInterface,
    public ModelInterface, public CorbaInterface
    {
      Q_OBJECT
        Q_INTERFACES (hpp::gui::PluginInterface
            hpp::gui::ModelInterface
            hpp::gui::CorbaInterface)

      public:
        struct JointElement {
          std::string name;
          std::string bodyName;
          JointTreeItem* item;
          bool updateViewer;

          JointElement ()
            : name (), bodyName (), item (NULL), updateViewer (false) {}
          JointElement (std::string n, std::string bn, JointTreeItem* i, bool updateV = true)
            : name (n), bodyName (bn), item (i), updateViewer (updateV) {}
        };
        typedef QMap <std::string, JointElement> JointMap;
        typedef hpp::corbaServer::Client HppClient;

        explicit HppWidgetsPlugin ();

        virtual ~HppWidgetsPlugin ();

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
        void configurationValidationStatus (QStringList collision);

        // CorbaInterface interface
      public:
        virtual bool corbaException (int jobId, const CORBA::Exception &excep) const;

        virtual void openConnection ();
        virtual void closeConnection ();
signals:
        void logJobFailed  (int id, const QString& text) const;

        public slots:
          void applyCurrentConfiguration ();
        void configurationValidation ();
        void selectJointFromBodyName (const QString bodyName);

      public:
        QList <QAction*> getJointActions (const std::string &jointName);

      public:
        HppClient* client () const;
        JointMap& jointMap ();
        PathPlayer* pathPlayer() const;

        virtual void updateRobotJoints (const QString robotName);
        std::string getSelectedJoint ();
        virtual Roadmap* createRoadmap (const std::string& jointName);

      signals:
        void logSuccess (const QString& text);
        void logFailure (const QString& text);

        protected slots:
          virtual void displayRoadmap (const std::string& jointName);
        void addJointFrame (const std::string& jointName);

      private:

        PathPlayer* pathPlayer_;
        SolverWidget* solverWidget_;
        ConfigurationListWidget* configListWidget_;
        QList <QDockWidget*> dockWidgets_;

        HppClient* hpp_;

      protected:
        static std::string escapeJointName (const std::string jn);
        std::string createJointGroup (const std::string jn);
        QString getIIOPurl () const;
        void computeObjectPosition();

        JointTreeWidget* jointTreeWidget_;
        JointMap jointMap_;
        std::list <std::string> jointFrames_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPWIDGETSPLUGIN_HH
