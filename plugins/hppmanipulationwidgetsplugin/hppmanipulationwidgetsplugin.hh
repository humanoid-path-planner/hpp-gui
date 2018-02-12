#ifndef HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH
#define HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH

#include <QToolBar>

#include <gepetto/gui/plugin-interface.hh>
#include <hpp/corbaserver/manipulation/client.hh>
#undef __robot_hh__
#undef __problem_hh__
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

class QListWidgetItem;

namespace hpp {
  namespace gui {
    /// HppManipulationWidgetsPlugin add functionalities to interact with hpp-manipulation-corba
    class HppManipulationWidgetsPlugin : public HppWidgetsPlugin
                                         // , public PluginInterface, public ModelInterface, public CorbaErrorInterface
    {
      Q_OBJECT
        Q_INTERFACES (gepetto::gui::PluginInterface
            gepetto::gui::ModelInterface
            gepetto::gui::CorbaInterface)

      public:
        typedef hpp::corbaServer::manipulation::Client HppManipClient;

        explicit HppManipulationWidgetsPlugin ();

        virtual ~HppManipulationWidgetsPlugin ();

        // PluginInterface interface
      public:
        /// Initialize the plugin
        void init();

        /// Return the name of the plugin
        QString name() const;

        // ModelInterface interface
      public:
        /// Load a robot in the manipulation server.
        /// \param rd definition of the robot (Name, package path, URDF filename, suffix)
        void loadRobotModel (gepetto::gui::DialogLoadRobot::RobotDefinition rd);

        /// Load an environment in the manipulation server.
        /// \param ed definition of the environment (Name, package path, URDF filename, suffix)
        void loadEnvironmentModel (gepetto::gui::DialogLoadEnvironment::EnvironmentDefinition ed);

        /// Get the name of a joint's body.
        /// \param jointName joint name
        std::string getBodyFromJoint (const std::string& jointName) const;

        // CorbaInterface
      public:
        /// Open a connection to a corba manipulation server.
        virtual void openConnection ();

        /// Close connection from corba manipulation server.
        virtual void closeConnection();

      public:
        /// Get the instance of corba manipulation client.
        HppManipClient* manipClient () const;

        /// Get the list of joints from corbaserver and update internal joint map.
        /// \param robotName name of the robot (unused)
        void updateRobotJoints (const QString robotName);

        /// Create the roadmap of a given joint.
        /// \param jointName name of the joint
        virtual Roadmap* createRoadmap (const std::string& jointName);

        public slots:
          /// Draw robot's contacts in the viewer.
          void drawRobotContacts ();

          /// Draw environment's contacts in the viewer.
          void drawEnvironmentContacts ();

          /// Draw handles frame in the viewer.
          void drawHandlesFrame ();

          /// Draw grippers frame in the viewer.
          void drawGrippersFrame ();

          /// Create the widget to select what to include in constraint's graph autobuild.
          void autoBuildGraph();

        private slots:
          /// Construct all the corba vars and create the graph.
          void buildGraph();
          void handleWorkerDone(int id);

      protected:
        virtual void loadConstraintWidget();

      private:
        // Type used to make one function to build datas needed for autoBuild
        typedef std::pair<hpp::Names_t, hpp::corbaserver::manipulation::Namess_t> NamesPair;
      typedef std::map<std::string, std::list<std::string> > MapNames;

        /// Convert a MapNames to a pair of corba types.
        NamesPair convertMap(MapNames& mapNames);

        /// Transform a list of QListWidgetItem to corba sequence of strings.
        hpp::Names_t_var convertToNames(const QList<QListWidgetItem *>& l);

      MapNames getObjects();
      void fillMap(MapNames& map, const QList<QListWidgetItem *>& l);
        void mergeShapes(MapNames& handles, MapNames& shapes);

      /// Draw the surface name describe by the points at index indexes[j] in points.
      /// The surface will be moved of epsilon along its normal so that it does not overlap
      /// others bodies.
      /// \param name surface name
      /// \param indexes table of indexes
      /// \param points table of points
      /// \param j index in table indexes
      /// \param epsilon offset
      void drawContactSurface(const std::string& name, hpp::intSeq_var& indexes,
			      hpp::floatSeqSeq_var& points, CORBA::ULong j, float epsilon = 0.0001f);

        HppManipClient* hpp_;

        QToolBar *toolBar_;
        QTabWidget *tw_;
        QDialog* graphBuilder_;

        int lastId_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_HPPMANIPULATIONWIDGETSPLUGIN_HH
