#ifndef ATTITUDEDEVICE_HH
#define ATTITUDEDEVICE_HH

#include <QObject>
#include <QDebug>
#include <QString>
#include <QHostInfo>
#include <QMessageBox>
#include <QtConcurrentRun>

#include <hpp/gui/deprecated.hh>

#include <remoteimu/mouse.hh>

#include <hpp/corbaserver/client.hh>


class AttitudeEventSender_deprecated : public QObject, public remoteimu::MouseEventSender
{
  Q_OBJECT

public:
  AttitudeEventSender_deprecated ();

  void mouseEvent(const Event e);

signals:
  void attitudeEvent (double w, double x, double y, double z);

private:
  hpp::Quaternion_ q;
} HPP_GUI_DEPRECATED;

class AttitudeDevice_deprecated : public QObject
{
  Q_OBJECT

public:
  AttitudeDevice_deprecated ();

  void init ();

  void jointName (const std::string jointName);

  AttitudeEventSender_deprecated* sender () {
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
  AttitudeEventSender_deprecated aes_;

  std::string jn;
  hpp::Quaternion_ q;
  hpp::boolSeq_var mask;

  float frameViz[7];

  QFuture <void> lock_;
} HPP_GUI_DEPRECATED;

class AttitudeDeviceMsgBox_deprecated : public QMessageBox
{
public:
  AttitudeDeviceMsgBox_deprecated (QWidget *parent);

  void setJointName (const std::string jn) {
    device_.jointName(jn);
  }

  void show ();

protected:
  virtual void keyPressEvent(QKeyEvent *event);

private:
  AttitudeDevice_deprecated device_;
} HPP_GUI_DEPRECATED;

#endif // ATTITUDEDEVICE_HH
