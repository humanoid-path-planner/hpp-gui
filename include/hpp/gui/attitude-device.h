#ifndef ATTITUDEDEVICE_HH
#define ATTITUDEDEVICE_HH

#include <QObject>
#include <QDebug>
#include <QtConcurrentRun>

#include <remoteimu/mouse.hh>

#include <hpp/corbaserver/client.hh>


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


public slots:
  void updateJointAttitude (double w, double x, double y, double z);
  void start ();
  void stop ();

private:
  remoteimu::Mouse mouse_;
  AttitudeEventSender aes_;

  std::string jn;
  hpp::Quaternion_ q;
  hpp::boolSeq_var mask;

  float frameViz[7];

  QFuture <void> lock_;
};

#endif // ATTITUDEDEVICE_HH
