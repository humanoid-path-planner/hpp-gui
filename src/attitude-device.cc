#include "hpp/gui/attitude-device.h"

#include <QtConcurrentRun>

#include "hpp/gui/mainwindow.h"

AttitudeEventSender::AttitudeEventSender(std::string jointName) :
  jn (jointName), q (), mask (new hpp::boolSeq)
{
  mask->length(3);
  mask[0]=true;mask[1]=true;mask[2]=true;
}

void AttitudeEventSender::mouseEvent(const remoteimu::MouseEventSender::Event e)
{
    qDebug () << e.ori[0] << ", " << e.ori[1] << ", " << e.ori[2];
    emit attitudeEvent(e.ori[0], e.ori[1], e.ori[2]);
    MainWindow* m = MainWindow::instance();

    hpp::Transform__var transform = m->hppClient()->robot()->getJointPosition(jn.c_str());
    hpp::floatSeq pos1, pos2; pos1.length(3); pos2.length(3);
    for (int i = 0; i < 4; i++) q[i] = e.q[i];
    for (int i = 0; i < 3; i++) pos2[i] = transform[i];
    for (int i = 0; i < 3; i++) pos1[i] = 0;

    m->hppClient()->problem()->createOrientationConstraint (
          "attitudeDeviceControl/ori", "", jn.c_str(),
          q, mask.in());
    m->hppClient()->problem()->createPositionConstraint (
          "attitudeDeviceControl/pos", "", jn.c_str(),
          pos1, pos2, mask.in());
    m->hppClient()->problem()->resetConstraints ();
    hpp::Names_t n; n.length(2);
    n[0] = "attitudeDeviceControl/ori"; n[1] = "attitudeDeviceControl/pos";
    m->hppClient()->problem()->setNumericalConstraints ("attitudeDeviceControl", n);
    hpp::floatSeq_var qin = m->hppClient()->robot()->getCurrentConfig ();
    hpp::floatSeq_var qproj;
    hpp::floatSeq_out qp_out (qproj);
    double err;
    bool res = m->hppClient()->problem()->applyConstraints (qin.in(), qp_out, err);
    if (!res) {
        qDebug() << "Projection failed: " << err;
        m->hppClient()->robot()->setCurrentConfig (qin.in());
    } else {
        qDebug() << "Projection succeeded";
        m->hppClient()->robot()->setCurrentConfig (qproj);
    }
    m->applyCurrentConfiguration();
}

AttitudeDevice::AttitudeDevice (std::string jointName) :
  mouse_ (new remoteimu::UDPServer ("0.0.0.0", 6000), 5),
  aes_ (jointName)
{
  mouse_.setMouseEventSender(&aes_);
}

void AttitudeDevice::start()
{
  if (lock_.isRunning()) stop ();
  lock_ = QtConcurrent::run (&mouse_, &remoteimu::Mouse::handleEvents, true);
}

void AttitudeDevice::stop()
{
  mouse_.stopEventHandler();
  lock_.waitForFinished();
}
