#ifndef HPP_GUI_REMOTEIMUPLUGIN_HH
#define HPP_GUI_REMOTEIMUPLUGIN_HH

#include <QObject>
#include <QDebug>
#include <QString>
#include <QHostInfo>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <QWidget>

#include <hpp/gui/plugin-interface.hh>

#include <iostream>

#include <remoteimu/mouse.hh>

#include <hpp/corbaserver/client.hh>


namespace hpp {
  namespace gui {
    class AttitudeEventSender : public QObject, public remoteimu::MouseEventSender
    {
      Q_OBJECT

      public:
        AttitudeEventSender ();

        void mouseEvent(const Event e);

      signals:
        void attitudeEvent (double w, double x, double y, double z);

      private:
        hpp::Quaternion_ q;
    };

    class AttitudeDevice : public QObject
    {
      Q_OBJECT

      public:
        AttitudeDevice ();

        void init ();

        void jointName (const std::string jointName);

        AttitudeEventSender* sender () {
          return &aes_;
        }

        QString address () const {
          return QString ("%1.%2:%3").arg(QHostInfo::localHostName(), QHostInfo::localDomainName(),
              QString::number (port));
        }

        public slots:
          void updateJointAttitude (double w, double x, double y, double z);
        void updateTargetPosition (double x, double y, double z);
        void start ();
        void stop ();

      private:
        const int port;

        remoteimu::Mouse mouse_;
        hpp::corbaServer::Client hpp_;
        AttitudeEventSender aes_;

        std::string jn;
        hpp::Quaternion_ q;
        hpp::boolSeq_var mask;

        float frameViz[7];

        QFuture <void> lock_;
    };

    class AttitudeDeviceMsgBox : public QMessageBox
    {
      public:
        AttitudeDeviceMsgBox (QWidget *parent);

        void setJointName (const std::string jn) {
          device_.jointName(jn);
        }

        void show ();

      protected:
        virtual void keyPressEvent(QKeyEvent *event);

      private:
        AttitudeDevice device_;
    };

    class RemoteImuPlugin : public QObject,
    public PluginInterface, public JointModifierInterface {
      Q_OBJECT
        Q_INTERFACES (hpp::gui::PluginInterface hpp::gui::JointModifierInterface)

      public:
        virtual ~RemoteImuPlugin ();

        void init();

        QString name () const;

        JointAction* action (const std::string &jointName) const;

        public slots:
          void newDevice (const std::string jointName);

      private:
        AttitudeDeviceMsgBox* msgBox_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_REMOTEIMUPLUGIN_HH
