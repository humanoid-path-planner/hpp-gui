#include <remoteimuplugin.hh>

#include <QtPlugin>
#include <QtConcurrentRun>

#include <hpp/gui/mainwindow.h>
#include <hpp/gui/windows-manager.h>

AttitudeEventSender::AttitudeEventSender()
{
}

void AttitudeEventSender::mouseEvent(const remoteimu::MouseEventSender::Event e)
{
  emit attitudeEvent(e.ori[0], e.ori[1], e.ori[2], e.ori[3]);
}

AttitudeDevice::AttitudeDevice () :
  port (6000),
  mouse_ (new remoteimu::UDPServer ("0.0.0.0", port), 5),
  hpp_ (0, 0),
  aes_ (), mask (new hpp::boolSeq)
{
  hpp_.connect();
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
  m->osg()->addBox("hpp-gui/attitudeControl", 0.001f, 0.001f, 0.001f, black);
  m->osg()->addLandmark("hpp-gui/attitudeControl", 0.1f);
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
  ModelInterface* model = m->pluginManager()->getFirstOf <ModelInterface> ();
  if (model == NULL) {
      qDebug () << "No ModelInterface found";
      return;
    }

  // Initialize the constraints
  hpp::Transform__var transform = hpp_.robot()->getJointPosition(jn.c_str());
  hpp::floatSeq pos1, pos2; pos1.length(3); pos2.length(3);
  for (int i = 0; i < 3; i++) pos1[i] = 0;
  for (int i = 0; i < 3; i++) pos2[i] = transform.in()[i];
  for (int i = 0; i < 3; i++) frameViz[i]=(float)pos2[i];
  frameViz [3] = 1;
  for (int i = 0; i < 3; i++) frameViz[i+4]=0;

  qDebug() << transform.in()[0+3] << ","
           << transform.in()[1+3] << ","
           << transform.in()[2+3] << ","
           << transform.in()[3+3];
  mouse_.setInitialQuat (Eigen::Quaterniond(transform.in()[0+3],
                                          transform.in()[1+3],
                                          transform.in()[2+3],
                                          transform.in()[3+3]));

  hpp_.problem()->createPositionConstraint (
        "attitudeDeviceControl/pos", "", jn.c_str(),
        pos2, pos1, mask.in());

  m->osg()->setVisibility("hpp-gui/attitudeControl", "ON");
  m->osg()->applyConfiguration("hpp-gui/attitudeControl", frameViz);
  std::string bodyName = model->getBodyFromJoint (jn);
  if (!jn.empty())
    m->osg()->addLandmark(bodyName.c_str (), 0.05f);
  m->osg()->refresh();
  lock_ = QtConcurrent::run (&mouse_, &remoteimu::Mouse::handleEvents, true);
}

void AttitudeDevice::stop()
{
  mouse_.stopEventHandler();
  MainWindow* m = MainWindow::instance();
  ModelInterface* model = m->pluginManager()->getFirstOf <ModelInterface> ();
  if (model == NULL) {
      qDebug () << "No ModelInterface found";
      return;
    }

  m->osg()->setVisibility("hpp-gui/attitudeControl", "OFF");
  std::string bodyName = model->getBodyFromJoint (jn);
  if (!jn.empty())
    m->osg()->deleteLandmark(bodyName.c_str ());
  lock_.waitForFinished();
}

void AttitudeDevice::updateJointAttitude(double w, double x, double y, double z)
{
  MainWindow* m = MainWindow::instance();

  q[0] = w; q[1] = x; q[2] = y; q[3] = z;
  for (int i = 0; i < 4; i++) {
      frameViz[i+3] = (float)q[i];
    }
  m->osg()->applyConfiguration("hpp-gui/attitudeControl", frameViz);

  hpp::floatSeq pos1, pos2; pos1.length(3); pos2.length(3);
  for (int i = 0; i < 3; i++) pos1[i] = 0;
  for (int i = 0; i < 3; i++) pos2[i] = frameViz[i];

  hpp_.problem()->createPositionConstraint (
        "attitudeDeviceControl/pos", "", jn.c_str(),
        pos2, pos1, mask.in());
  hpp_.problem()->createOrientationConstraint (
        "attitudeDeviceControl/ori", "", jn.c_str(),
        q, mask.in());
  hpp_.problem()->resetConstraints ();
  hpp::Names_t n; n.length(2);
  n[0] = "attitudeDeviceControl/ori"; n[1] = "attitudeDeviceControl/pos";
  hpp_.problem()->setNumericalConstraints ("attitudeDeviceControl", n);
  hpp::floatSeq_var qin = hpp_.robot()->getCurrentConfig ();
  hpp::floatSeq_var qproj;
  hpp::floatSeq_out qp_out (qproj);
  double err;
  bool res = hpp_.problem()->applyConstraints (qin.in(), qp_out, err);
  if (!res) {
      qDebug() << "Projection failed: " << err;
      hpp_.robot()->setCurrentConfig (qin.in());
  } else {
      hpp_.robot()->setCurrentConfig (qproj);
  }
  m->requestApplyCurrentConfiguration();
}

void AttitudeDevice::updateTargetPosition (double x, double y, double z) {
  Eigen::Quaternion<double> q (frameViz[3], frameViz[4], frameViz[5], frameViz[6]);
  Eigen::Vector3d dx = q.inverse() * Eigen::Vector3d (x,y,z);
  frameViz[0] += (float)dx[0];
  frameViz[1] += (float)dx[1];
  frameViz[2] += (float)dx[2];
}

AttitudeDeviceMsgBox::AttitudeDeviceMsgBox (QWidget *parent) :
   QMessageBox (QMessageBox::Information,  "Attitude Device","",
                QMessageBox::Close, parent,
                Qt::Dialog | Qt::WindowStaysOnTopHint),
  device_ ()
{
  setModal (false);
  setText ("Configuration steps:\n"
           "1 - Configure your device to send its datas to " + device_.address() + "\n"
           "2 - Do not move your device for a short initialization time (corresponding to 100 measurements of your device).\n"
           "3 - Move your device, the frame should move as well.\n"
           "4 - Close this popup to stop the connection."
           );
}

void AttitudeDeviceMsgBox::show()
{
  connect (this, SIGNAL(finished(int)), &device_, SLOT (stop()));
  connect (this, SIGNAL(finished(int)), this, SLOT (deleteLater()));
  device_.init ();
  device_.start ();
  QMessageBox::show ();
}

void AttitudeDeviceMsgBox::keyPressEvent(QKeyEvent *event)
{
  double shift = 0.01;
  if (event->modifiers() == Qt::ControlModifier)
    shift /= 10;
  else if (event->modifiers() == Qt::ShiftModifier)
    shift *= 10;
  switch (event->key ()) {
    case Qt::Key_Up:
    case Qt::Key_W:
      qDebug () << "X+";
      device_.updateTargetPosition (shift, 0, 0);
      break;
    case Qt::Key_Down:
    case Qt::Key_S:
      qDebug () << "X-";
      device_.updateTargetPosition (-shift, 0, 0);
      break;
    case Qt::Key_Left:
    case Qt::Key_A:
      qDebug () << "Y+";
      device_.updateTargetPosition (0, shift, 0);
      break;
    case Qt::Key_Right:
    case Qt::Key_D:
      qDebug () << "Y-";
      device_.updateTargetPosition (0, -shift, 0);
      break;
    case Qt::Key_PageUp:
    case Qt::Key_Q:
      qDebug () << "Z+";
      device_.updateTargetPosition (0, 0, shift);
      break;
    case Qt::Key_PageDown:
    case Qt::Key_E:
      qDebug () << "Z-";
      device_.updateTargetPosition (0, 0, -shift);
      break;
    }

  QMessageBox::keyPressEvent(event);
}

RemoteImuPlugin::~RemoteImuPlugin()
{
}

void RemoteImuPlugin::init() {
  msgBox_ = NULL;
}

QString RemoteImuPlugin::name() const
{
  return QString ("Remote IMU");
}

QAction* RemoteImuPlugin::action(const std::string &jointName) const
{
  QAction* action = new QAction (QIcon::fromTheme("smartphone"), "Attach to attitude device", NULL);
  action->setObjectName(QString::fromStdString(jointName));
  connect (action, SIGNAL (triggered()), this, SLOT (newDevice ()));
  return action;
}

void RemoteImuPlugin::newDevice()
{
  QString name = QObject::sender()->objectName();
  msgBox_ = new AttitudeDeviceMsgBox (NULL);
  msgBox_->setJointName (name.toStdString());
  msgBox_->show();
}

Q_EXPORT_PLUGIN2 (remoteimuplugin, RemoteImuPlugin)
