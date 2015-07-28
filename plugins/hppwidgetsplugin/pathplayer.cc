#include "pathplayer.h"

#include <hpp/corbaserver/common.hh>
#include <hpp/corbaserver/client.hh>

#include "hpp/gui/mainwindow.h"
#include <hpp/gui/windows-manager.h>
#include <hpp/gui/osgwidget.h>

#include "ui_pathplayerwidget.h"

PathPlayer::PathPlayer (HppWidgetsPlugin *plugin, QWidget *parent) :
  QWidget (parent)
, ui_ (new Ui::PathPlayerWidget)
, frameRate_ (25)
, plugin_ (plugin)
{
  ui_->setupUi (this);
  pathIndex()->setMaximum(0);
  connect (pathSlider(), SIGNAL (sliderMoved (int)), this, SLOT (pathSliderChanged(int)));
  connect (pathIndex(), SIGNAL (valueChanged(int)), this, SLOT (pathIndexChanged(int)));
  connect (playPause(), SIGNAL (toggled (bool)), this, SLOT (playPauseToggled(bool)));
  connect (stop(), SIGNAL (clicked()), this, SLOT (stopClicked()));
  connect (record(), SIGNAL (toggled(bool)), this, SLOT (recordToggled(bool)));
  connect (ui_->refreshButton_path, SIGNAL (clicked()), this, SLOT (update()));
}

PathPlayer::~PathPlayer()
{
  delete ui_;
}

void PathPlayer::update ()
{
  CORBA::Short nbPath = plugin_->client()->problem ()->numberPaths ();
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
          pathIndex()->setValue(0);
        }
      pathIndex()->setMaximum(nbPath - 1);
    }
  else {
      pathIndex()->setEnabled(false);
      pathSlider()->setEnabled(false);
      playPause()->setEnabled(false);
      stop()->setEnabled(false);
    }
}

void PathPlayer::pathIndexChanged(int i)
{
  assert (i >= 0);
  pathLength_ = plugin_->client()->problem()->pathLength ((short unsigned int)i);
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
  if (toggled) timerId_ = startTimer(0);
  else killTimer(timerId_);
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

void PathPlayer::recordToggled(bool toggled)
{
  if (toggled) {
      std::string path = "/tmp/hpp-gui/record/img";
      std::string ext = "jpeg";
      MainWindow::instance()->osg ()->startCapture(
            MainWindow::instance()->centralWidget()->windowID(),
            path.c_str(),
            ext.c_str());
    }
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
      else
        timerId_ = startTimer(timeBetweenRefresh());
      pathSlider()->setSliderPosition(lengthToSlider(currentParam_));
      updateConfiguration();
    }
}

void PathPlayer::timerEvent(QTimerEvent *event)
{
  if (timerId_ == event->timerId()) {
      pathPulse();
    }
}

void PathPlayer::updateConfiguration ()
{
  hpp::floatSeq_var config =
      plugin_->client()->problem()->configAtParam ((short unsigned int)pathIndex()->value(),currentParam_);
  plugin_->client()->robot()->setCurrentConfig (config.in());
  MainWindow::instance()->requestApplyCurrentConfiguration();
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
  return time * pathLength_ / timeSpinBox()->value();
}

inline int PathPlayer::timeBetweenRefresh() const
{
  return 1000/frameRate_;
}

double PathPlayer::lengthBetweenRefresh() const
{
  return pathLength_ / (timeSpinBox()->value() * frameRate_);
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

QPushButton *PathPlayer::record() const
{
  return ui_->recordButton;
}
