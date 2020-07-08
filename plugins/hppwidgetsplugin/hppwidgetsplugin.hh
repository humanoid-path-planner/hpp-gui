//
// Copyright (c) CNRS
// Author: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_HPPWIDGETSPLUGIN_HH
#define HPP_GUI_HPPWIDGETSPLUGIN_HH

#include <gepetto/gui/plugin-interface.hh>
#include <gepetto/gui/windows-manager.hh>
#include <hpp/corbaserver/client.hh>

class QDockWidget;

namespace hpp {
  namespace gui {
    class SolverWidget;
    class PathPlayer;
    class JointTreeWidget;
    class ConfigurationListWidget;
    class JointTreeItem;
    class Roadmap;
    class ConstraintWidget;
    class DemoSubWidget;

    inline CORBA::String_var to_corba(const QString& s)
    { return (const char*)s.toLocal8Bit().data(); }

    /// Plugin that add a lot of features to work with hpp.
    class HppWidgetsPlugin : public QObject, public gepetto::gui::PluginInterface,
    public gepetto::gui::ModelInterface, public gepetto::gui::ConnectionInterface
    {
      Q_OBJECT
        Q_INTERFACES (gepetto::gui::PluginInterface
            gepetto::gui::ModelInterface
            gepetto::gui::ConnectionInterface)

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    Q_PLUGIN_METADATA (IID "hpp-gui.hppwidgetsplugin")
#endif

      public:
        struct JointElement {
          std::string name, prefix;
          // FIXME sort this vector.
          std::vector<std::string> bodyNames;
          JointTreeItem* item;
          std::vector<bool> updateViewer;

          JointElement ()
            : name (), bodyNames (), item (NULL), updateViewer (0, false) {}
          JointElement (const std::string& n,
              const std::string& prefix,
              const std::vector<std::string>& bns,
              JointTreeItem* i,
              bool updateV = true)
            : name (n), prefix (prefix), bodyNames (bns), item (i),
            updateViewer (bns.size(), updateV) {}
          JointElement (const std::string& n, const std::string& prefix,
              const hpp::Names_t& bns, JointTreeItem* i, bool updateV = true);
        };
        typedef QMap <std::string, JointElement> JointMap;
        typedef hpp::corbaServer::Client HppClient;

        explicit HppWidgetsPlugin ();

        virtual ~HppWidgetsPlugin ();

        // PluginInterface interface
      public:
        /// Initialize the plugin.
        void init();

        /// Returns the plugin's name.
        QString name() const;

        // ModelInterface interface
      public:
        /// Load a robot in the corba server.
        /// \param rd robot definition
        void loadRobotModel (gepetto::gui::DialogLoadRobot::RobotDefinition rd);

        /// Load an environment in the corba server.
        /// \param ed environment definition
        void loadEnvironmentModel (gepetto::gui::DialogLoadEnvironment::EnvironmentDefinition ed);

        /// Get the name of a joint's body.
        /// \param jointName joint name
        /// \todo this should be changed because there can be several body per
        /// joints now.
        std::string getBodyFromJoint (const std::string& jointName) const;

        const hpp::floatSeq& currentConfig () const
        {
          return config_;
        }

        hpp::floatSeq& currentConfig ()
        {
          return config_;
        }

        const hpp::floatSeq& currentVelocity () const
        {
          return velocity_;
        }

        hpp::floatSeq& currentVelocity ()
        {
          return velocity_;
        }

signals:
        void configurationValidationStatus (bool valid);
        void configurationValidationStatus (QStringList collision);

        // ConnectionInterface interface
      public:
        /// Open a connection to a corba server.
        virtual void openConnection ();

        /// Close connection to corbaserver.
        virtual void closeConnection ();
signals:
        /// Log the failure of a job in the MainWindow.
        void logJobFailed  (int id, const QString& text) const;

        public slots:
        /// Apply the current configuration of the robot.
          void applyCurrentConfiguration ();

        void setCurrentConfig (const hpp::floatSeq& q);

        hpp::floatSeq const* getCurrentConfig () const;

        void setCurrentQtConfig (const QVector<double>& q);

        QVector<double> getCurrentQtConfig () const;

        /// Set internal configuration from HPP current config.
        void fetchConfiguration ();

        /// Set HPP configuration to internal current configuration
        void sendConfiguration ();

        /// Build a list of bodies in collision.
        void configurationValidation ();

        /// Select a joint in the joint tree from a body's name.
        /// \param bodyName name of the body
        void selectJointFromBodyName (const QString bodyName);

        void update();

        /// See createJointGroup
        QString requestCreateJointGroup(const QString jn);

        /// See createComGroup
        QString requestCreateComGroup(const QString com);

        QString getHppIIOPurl () const;

        QString getHppContext () const;

      public:
        /// Get the corbaserver client.
        HppClient* client () const;

        /// Get the jointMap.
        JointMap& jointMap ();

        /// Get the pathPlayer widget.
        PathPlayer* pathPlayer() const;

        /// Get the pathPlayer widget.
        JointTreeWidget* jointTreeWidget() const;

        /// Get the pathPlayer widget.
        ConfigurationListWidget* configurationListWidget() const;

        /// Get the list of joints from corbaserver and update internal joint map.
        /// \param robotName name of the robot
        virtual void updateRobotJoints (const QString robotName);

        /// Get the currently selected joint name.
        std::string getSelectedJoint () const;

        /// Create the roadmap of a given joint.
        /// \param jointName name of the joint
        virtual Roadmap* createRoadmap (const std::string& jointName);

      signals:
        void logSuccess (const QString& text);
        void logFailure (const QString& text);

        protected slots:
        /// Display the roadMap of a given joint.
        /// \param jointName name of the joint
          virtual void displayRoadmap (const std::string& jointName);

        /// Add XYZ axis at the joint's position
        /// \param jointName name of the joint
        void addJointFrame (const std::string& jointName);

      private:
        void prepareApplyConfiguration ();

        PathPlayer* pathPlayer_;
        SolverWidget* solverWidget_;
        ConfigurationListWidget* configListWidget_;

        HppClient* hpp_;

      protected:
        /// Change all "/" in jn to "__"
        /// \param jn string to escape
        static std::string escapeJointName (const std::string jn);

        /// Create a group from the given joint.
        /// \param jn joint name
        std::string createJointGroup (const std::string jn);

        /// Create a group from the given COM.
        /// \param com COM name
        std::string createComGroup (const std::string com);

        /// Replace all the bodies according to their position in hpp.
        void computeObjectPosition();

        virtual void loadConstraintWidget();

        QList <QDockWidget*> dockWidgets_;
        JointTreeWidget* jointTreeWidget_;
        ConstraintWidget* constraintWidget_;
        JointMap jointMap_;
        hpp::Names_t jointFrames_;
        std::list <std::string> comFrames_;

        DemoSubWidget* demoSubWidget_; 

        hpp::floatSeq config_, velocity_;

        // Cache variables
        hpp::Names_t             linkNames_;
        std::vector<std::string> bodyNames_;
        std::vector<gepetto::viewer::Configuration> bodyConfs_;
        std::vector<std::string> jointGroupNames_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPWIDGETSPLUGIN_HH
