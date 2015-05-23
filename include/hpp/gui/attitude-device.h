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
  AttitudeEventSender (std::string jointName);

  void mouseEvent(const Event e);

signals:
  void attitudeEvent (double r, double p, double y);

private:
  std::string jn;
  hpp::Quaternion_ q;
  hpp::boolSeq_var mask;
};

class AttitudeDevice
{
public:
  AttitudeDevice (std::string jointName);

  AttitudeEventSender* sender () {
    return &aes_;
  }

  void start ();

  void stop ();

private:
  remoteimu::Mouse mouse_;
  AttitudeEventSender aes_;

  QFuture <void> lock_;
};

#endif // ATTITUDEDEVICE_HH
