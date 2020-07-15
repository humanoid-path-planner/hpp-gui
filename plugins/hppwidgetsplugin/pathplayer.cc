//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#include "hppwidgetsplugin/pathplayer.hh"

#include <assert.h>
#include <sstream>

#include <QProgressDialog>
#include <QSlider>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
# include <QtCore>
#else
# include <QtConcurrent>
#endif

#include <hpp/corbaserver/client.hh>

#include "gepetto/gui/mainwindow.hh"
#include <gepetto/gui/windows-manager.hh>
#include <gepetto/gui/osgwidget.hh>
#include <gepetto/gui/action-search-bar.hh>

#include "hppwidgetsplugin/conversions.hh"
#include "hppwidgetsplugin/ui_pathplayerwidget.h"
#include "hppwidgetsplugin/joint-action.hh"
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace hpp {
  namespace gui {
    using gepetto::gui::MainWindow;
    using gepetto::gui::ActionSearchBar;

    PathPlayer::PathPlayer (HppWidgetsPlugin *plugin, QWidget *parent) :
      QWidget (parent)
      , ui_ (new ::Ui::PathPlayerWidget)
      , timerId_ (0)
      , velocity_ (false)
      , plugin_ (plugin)
    {
      ui_->setupUi (this);
      pathIndex()->setMaximum(0);
      connect (pathSlider(), SIGNAL (sliderMoved (int)), this, SLOT (pathSliderChanged(int)));
      connect (pathIndex(), SIGNAL (valueChanged(int)), this, SLOT (pathIndexChanged(int)));
      connect (playPause(), SIGNAL (toggled (bool)), this, SLOT (playPauseToggled(bool)));
      connect (stop(), SIGNAL (clicked()), this, SLOT (stopClicked()));
      connect (ui_->refreshButton_path, SIGNAL (clicked()), this, SLOT (update()));

      initSearchActions();
    }

    PathPlayer::~PathPlayer()
    {
      delete ui_;
    }

    void PathPlayer::initSearchActions()
    {
      ActionSearchBar* asb = MainWindow::instance()->actionSearchBar();
      JointAction* a;

      a = new JointAction (tr("Display &waypoints of selected path"), plugin_->jointTreeWidget(), this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (displayWaypointsOfPath(std::string)));
      asb->addAction(a);

      a = new JointAction (tr("Display selected &path"), plugin_->jointTreeWidget(), this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (displayPath(std::string)));
      asb->addAction(a);

      {
        QAction* a = new QAction (tr("Play path"), this);
        playPause()->connect (a, SIGNAL (triggered()), SLOT (toggle()));
        asb->addAction(a);
      }
    }

    void PathPlayer::displayWaypointsOfPath(const std::string jointName)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      if (!pathIndex()->isEnabled()) {
        main->logError("There is no path. Did you solve a problem ?");
        return;
      }
      int pid = pathIndex()->value();
      std::stringstream ss; ss << "path" << pid << "_" << jointName;
      std::string pn = ss.str();
      const osgVector4 colorN (0.f, 0.f, 1.f, 1.f),
                       colorE (1.f, 0.f, 0.f, 1.f);
      gepetto::gui::WindowsManagerPtr_t wsm = main->osg();
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeq_var times;
      hpp::floatSeqSeq_var waypoints = hpp->problem()->getWaypoints((CORBA::ULong)pid, times.out());
      if (wsm->nodeExists(pn)) wsm->deleteNode (pn, true);
      wsm->createGroup (pn);
      wsm->addToGroup(pn, "hpp-gui");
      // Temporary object to avoid dynamic allocation.
      // Arguments are max, length, storage, take ownership.
      char* tmps[1];
      hpp::Names_t names (1, 1, tmps, false);
      gepetto::viewer::Configuration pos;
      ::osg::Vec3ArrayRefPtr posSeq = new ::osg::Vec3Array;
      for (unsigned int i = 0; i < waypoints->length(); ++i) {
        // Make name
        ss.clear(); ss.str(std::string()); ss << pn << "/node" << i;
        std::string xyzName = ss.str();
        // Get positions
        names[0] = jointName.c_str();
        hpp::TransformSeq_var Ts = hpp->robot()->getJointsPosition(waypoints[i], names);
        fromHPP(Ts[0], pos);
        posSeq->push_back(pos.position);
        // Create the nodes
        if (wsm->nodeExists(xyzName)) wsm->deleteNode(xyzName, false);
        wsm->addXYZaxis(xyzName, colorN, 0.01f, 0.05f);
        wsm->applyConfiguration(xyzName, pos);
      }
      wsm->addCurve(pn + "/curve", posSeq, colorE);
      wsm->refresh();
    }

    void PathPlayer::displayPath(const std::string jointName)
    {
      QFutureWatcher <void>* fw = new QFutureWatcher<void>(this);
      QProgressDialog* pd = new QProgressDialog ("Computing curved path. Please wait...", "Cancel", 0, 100, this);
      connect(this, SIGNAL(displayPath_status(int)), pd, SLOT (setValue(int)));
      pd->setCancelButton(0);
      pd->setRange(0, 100);
      pd->show();
      fw->setFuture(QtConcurrent::run (this, &PathPlayer::displayPath_impl, jointName));
      connect (fw, SIGNAL (finished()), pd, SLOT (deleteLater()));
      connect (fw, SIGNAL (finished()), fw, SLOT (deleteLater()));
    }

    void PathPlayer::displayPath_impl(const std::string jointName)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      if (!pathIndex()->isEnabled()) {
        main->logError("There is no path. Did you solve a problem ?");
        emit displayPath_status(100);
        return;
      }
      CORBA::ULong pid = (CORBA::ULong) pathIndex()->value();
      std::stringstream ss; ss << "curvedpath_" << pid << "_" << jointName;
      std::string pn = ss.str();
      gepetto::viewer::WindowsManager::Color_t colorE (1.f, 0.f, 0.f, 1.f);
      gepetto::gui::WindowsManagerPtr_t wsm = main->osg();
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      CORBA::Double length = hpp->problem()->pathLength(pid);
      double dt = lengthBetweenRefresh();
      CORBA::ULong nbPos = (CORBA::ULong)(length / dt) + 1;
      osgVector3 pos;
      ::osg::Vec3ArrayRefPtr posSeq = new ::osg::Vec3Array;
      // Temporary object to avoid dynamic allocation.
      // Arguments are max, length, storage, take ownership.
      char* tmps[1];
      hpp::Names_t names (1, 1, tmps, false);
      float statusStep = 100.f / (float) nbPos;
      emit displayPath_status(0);
      for (CORBA::ULong i = 0; i < nbPos; ++i) {
        double t = std::min (i * dt, length);
        hpp::floatSeq_var q = hpp->problem()->configAtParam(pid, t);
        names[0] = jointName.c_str();
        hpp::TransformSeq_var Ts = hpp->robot()->getJointsPosition(q.in(), names);
        fromHPP(Ts[0], pos);
        posSeq->push_back(pos);
        emit displayPath_status(qFloor ((float)i * statusStep));
      }
      if (wsm->nodeExists(pn)) wsm->deleteNode (pn, true);
      wsm->addCurve(pn, posSeq, colorE);
      wsm->addToGroup(pn.c_str(), "hpp-gui");
      wsm->refresh();
      emit displayPath_status (100);
    }

    void PathPlayer::update ()
    {
      CORBA::Long nbPath = plugin_->client()->problem ()->numberPaths ();
      if (nbPath > 0)
      {
        pathIndex()->setEnabled(true);
        pathSlider()->setEnabled(true);
        playPause()->setEnabled(true);
        stop()->setEnabled(true);
        if (pathIndex()->maximum() == 0) {
          // If path index value is 0, no signal valueChanged will
          // be emitted. Force a value changed.
          if (pathIndex()->value() == 0)
            pathIndexChanged(0);
        }
        pathIndex()->setMaximum(nbPath - 1);
	pathIndex()->setValue(nbPath - 1);
      }
      else {
        pathIndex()->setEnabled(false);
        pathSlider()->setEnabled(false);
        playPause()->setEnabled(false);
        stop()->setEnabled(false);
      }
    }

    void PathPlayer::setRobotVelocity (bool set)
    {
      velocity_ = set;
    }

    void PathPlayer::pathIndexChanged(int i)
    {
      assert (i >= 0);
      pathLength_ = plugin_->client()->problem()->pathLength ((short unsigned int)i);
      timeSpinBox()->setValue(pathLength_);
      currentParam_= 0;
    }

    void PathPlayer::pathSliderChanged(int value)
    {
      // The user moved the slider manually.
      currentParam_ = sliderToLength(value);
      updateConfiguration();
    }

    void PathPlayer::playPauseToggled(bool toggled)
    {
      if (toggled) {
        if (timerId_ == 0) timerId_ = startTimer(timeBetweenRefresh());
      } else {
        killTimer(timerId_);
        timerId_ = 0;
      }
    }

    void PathPlayer::stopClicked()
    {
      // Pause
      playPause()->setChecked(false);
      // Reset current position.
      currentParam_ = 0;
      pathSlider()->setSliderPosition(0);
      updateConfiguration();
    }

    void PathPlayer::pathPulse()
    {
      if (playPause()->isChecked()) {
        // Stil playing
        currentParam_ += lengthBetweenRefresh();
        if (currentParam_ > pathLength_) {
          currentParam_ = pathLength_;
          playPause()->setChecked(false);
        }
        pathSlider()->setSliderPosition(lengthToSlider(currentParam_));
        updateConfiguration();
      }
    }

    void PathPlayer::setCurrentTime (const double& param)
    {
      if (!pathIndex()->isEnabled()) return;
      if (currentParam_ > pathLength_)
        currentParam_ = pathLength_;
      if (0 < currentParam_)
        currentParam_ = 0;
      currentParam_ = param;
      pathSlider()->setSliderPosition(lengthToSlider(currentParam_));
      updateConfiguration();
    }

    void PathPlayer::timerEvent(QTimerEvent *event)
    {
      if (timerId_ == event->timerId()) {
        pathPulse();
      }
    }

    void PathPlayer::updateConfiguration ()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      hpp::floatSeq_var config, velocity;
      try {
        config = plugin_->client()->problem()->configAtParam
          ((CORBA::ULong)pathIndex()->value(),currentParam_);
        if (velocity_)
          velocity = plugin_->client()->problem()->derivativeAtParam
            ((CORBA::ULong)pathIndex()->value(),1,currentParam_);
      } catch (const hpp::Error& e) {
        main->logError(e.msg.in());
        return;
      }
      plugin_->currentConfig() = config.in();
      if (velocity_)
        plugin_->currentVelocity() = velocity.in();
      main->requestApplyCurrentConfiguration();
      emit appliedConfigAtParam (getCurrentPath(), currentParam_);
    }

    inline double PathPlayer::sliderToLength(int v) const
    {
      return ((double)v / (double)pathSlider()->maximum()) * pathLength_;
    }

    int PathPlayer::lengthToSlider(double l) const
    {
      return (int) (pathSlider()->maximum() * l / pathLength_);
    }

    inline double PathPlayer::timeToLength(double time) const
    {
      if (timeSpinBox()->value() == 0) return time;
      return time * pathLength_ / timeSpinBox()->value();
    }

    inline int PathPlayer::timeBetweenRefresh() const
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      return main->settings_->refreshRate;
    }

    double PathPlayer::lengthBetweenRefresh() const
    {
      if (timeSpinBox()->value() == 0) return timeBetweenRefresh() / 1000;
      return pathLength_ * timeBetweenRefresh() / (1000 * timeSpinBox()->value());
    }

    QDoubleSpinBox *PathPlayer::timeSpinBox() const
    {
      return ui_->timeSpinBox;
    }

    QSpinBox *PathPlayer::pathIndex() const
    {
      return ui_->pathIndexSpin;
    }

    QSlider *PathPlayer::pathSlider() const
    {
      return ui_->pathSlider;
    }

    QPushButton *PathPlayer::playPause() const
    {
      return ui_->playPauseButton;
    }

    QPushButton *PathPlayer::stop() const
    {
      return ui_->stopButton;
    }

    int	PathPlayer::getCurrentPath() const
    {
      if (!pathIndex()->isEnabled()) return -1;
      return pathIndex()->value();
    }
  } // namespace gui
} // namespace hpp
