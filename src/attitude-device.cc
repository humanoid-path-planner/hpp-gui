#include "hpp/gui/attitude-device.h"

#include <QtConcurrentRun>

#include "hpp/gui/mainwindow.h"

AttitudeEventSender::AttitudeEventSender()
{
}

void AttitudeEventSender::mouseEvent(const remoteimu::MouseEventSender::Event e)
{
  emit attitudeEvent(e.ori[0], e.ori[1], e.ori[2], e.ori[3]);
}

AttitudeDevice::AttitudeDevice () :
  mouse_ (new remoteimu::UDPServer ("0.0.0.0", 6000), 5),
  aes_ (), mask (new hpp::boolSeq)
{
}

void AttitudeDevice::init ()
{
  mouse_.setMouseEventSender(&aes_);

  mask->length(3);
  mask[0]=true;mask[1]=true;mask[2]=true;

  connect(&aes_, SIGNAL (attitudeEvent (double, double, double, double)),
          this, SLOT (updateJointAttitude (double, double, double, double)));

  MainWindow* m = MainWindow::instance();
  float black[4] = {1,1,1,1};
  m->osg()->addBox("hpp-gui/attitudeControl", 0.001, 0.001, 0.001, black);
  m->osg()->addLandmark("hpp-gui/attitudeControl", 0.05f);
  m->osg()->setVisibility("hpp-gui/attitudeControl", "OFF");
}

void AttitudeDevice::jointName (const std::string jointName)
{
  jn = jointName;
}

void AttitudeDevice::start()
{
  if (lock_.isRunning()) stop ();

  MainWindow* m = MainWindow::instance();

  // Initialize the constraints
  hpp::Transform__var transform = m->hppClient()->robot()->getJointPosition(jn.c_str());
  hpp::floatSeq pos1, pos2; pos1.length(3); pos2.length(3);
  for (int i = 0; i < 3; i++) pos1[i] = 0;
  for (int i = 0; i < 3; i++) pos2[i] = transform.in()[i];
  for (int i = 0; i < 3; i++) frameViz[i]=(float)pos2[i];
  frameViz [3] = 1;
  for (int i = 0; i < 3; i++) frameViz[i+4]=0;

  m->hppClient()->problem()->createPositionConstraint (
        "attitudeDeviceControl/pos", "", jn.c_str(),
        pos2, pos1, mask.in());

  m->osg()->setVisibility("hpp-gui/attitudeControl", "ON");
  m->osg()->applyConfiguration("hpp-gui/attitudeControl", frameViz);
  MainWindow::JointMap::const_iterator it = m->jointMap ().find (jn);
  if (it != m->jointMap ().end())
    m->osg()->addLandmark(it->bodyName.c_str (), 0.05f);
  m->osg()->refresh();
  lock_ = QtConcurrent::run (&mouse_, &remoteimu::Mouse::handleEvents, true);
}

void AttitudeDevice::stop()
{
  mouse_.stopEventHandler();
  MainWindow* m = MainWindow::instance();
  m->osg()->setVisibility("hpp-gui/attitudeControl", "OFF");
  MainWindow::JointMap::const_iterator it = m->jointMap ().find (jn);
  if (it != m->jointMap ().end())
    m->osg()->deleteLandmark(it->bodyName.c_str ());
  lock_.waitForFinished();
}

void AttitudeDevice::updateJointAttitude(double w, double x, double y, double z)
{
  qDebug () << w << ", " << x << ", " << y << ", " << z;

  MainWindow* m = MainWindow::instance();

  q[0] = w; q[1] = x; q[2] = y; q[3] = z;
  for (int i = 0; i < 4; i++) {
      frameViz [i+3] = q [i];
    }

  m->osg()->applyConfiguration("hpp-gui/attitudeControl", frameViz);
  m->hppClient()->problem()->createOrientationConstraint (
        "attitudeDeviceControl/ori", "", jn.c_str(),
        q, mask.in());
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
