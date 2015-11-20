#ifndef HPP_GUI_PLUGININTERFACE_HH
#define HPP_GUI_PLUGININTERFACE_HH

#include <QtGui>
#include <QWidget>

#include <hpp/gui/dialog/dialogloadrobot.hh>
#include <hpp/gui/dialog/dialogloadenvironment.hh>

#include <omniORB4/CORBA.h>

namespace hpp {
  namespace gui {
    const int DockKeyShortcutBase = Qt::CTRL + Qt::ALT;

    class PluginInterface {
      public:
        PluginInterface ()
          : errorMsg_ ("Not initalized")
          , isInit_ (false)
        {}

        virtual ~PluginInterface () {}

        virtual QString name () const = 0;

        void doInit ()
        {
          try {
            init ();
            isInit_ = true;
          } catch (const std::exception& e) {
            errorMsg_ = QString (e.what ());
          }
        }

        bool isInit () const
        {
          return isInit_;
        }

        const QString& errorMsg () const
        {
          return errorMsg_;
        }

      protected:
        virtual void init () = 0;

      private:
        QString errorMsg_;
        bool isInit_;
    };

      class JointAction : public QAction {
      Q_OBJECT

      public:
        JointAction (const QString& actionName, const std::string& jointName, QObject* parent)
          : QAction (actionName, parent)
            , jointName_ (jointName)
      {
        connect (this, SIGNAL (triggered(bool)), SLOT(trigger()));
      }

signals:
        void triggered (const std::string jointName);

        private slots:
          void trigger () {
            emit triggered(jointName_);
          }

      private:
        const std::string jointName_;
    };

    class JointModifierInterface {
      public:
        virtual ~JointModifierInterface () {}

        virtual JointAction* action (const std::string& jointName) const = 0;
    };

      class ModelInterface {
        public:
          virtual ~ModelInterface () {}

          virtual void loadRobotModel (DialogLoadRobot::RobotDefinition rd) = 0;

          virtual void loadEnvironmentModel (DialogLoadEnvironment::EnvironmentDefinition ed) = 0;

          virtual std::string getBodyFromJoint (const std::string& jointName) const = 0;
      };

      class CorbaInterface {
        public:
          virtual ~CorbaInterface () {}

          virtual void openConnection () = 0;

          virtual void closeConnection () = 0;

          /// return true if error was handled.
          virtual bool corbaException (int jobId, const CORBA::Exception& excep) const = 0;
      };
  } // namespace gui
} // namespace hpp

Q_DECLARE_INTERFACE (hpp::gui::PluginInterface, "hpp-gui.plugins/0.0")
Q_DECLARE_INTERFACE (hpp::gui::JointModifierInterface, "hpp-gui.plugin.joint-modifier/0.0")
Q_DECLARE_INTERFACE (hpp::gui::ModelInterface, "hpp-gui.plugin.model/0.0")
Q_DECLARE_INTERFACE (hpp::gui::CorbaInterface, "hpp-gui.plugin.corba/0.0")



#endif // HPP_GUI_PLUGININTERFACE_HH
