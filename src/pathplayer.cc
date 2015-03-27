#include "hpp/gui/pathplayer.h"

#include <hpp/corbaserver/common.hh>
#include <hpp/corbaserver/client.hh>

#include "hpp/gui/mainwindow.h"
#include "hpp/gui/solverwidget.h"

PathPlayer::PathPlayer (QWidget *parent) :
  QWidget (parent)
, main_ (MainWindow::instance())
, frameRate_ (25)
{}

PathPlayer::~PathPlayer()
{
}

void PathPlayer::setup()
{
  pathIndex()->setMaximum(0);
  connect (pathSlider(), SIGNAL (sliderMoved (int)), this, SLOT (pathSliderChanged(int)));
  connect (pathIndex(), SIGNAL (valueChanged(int)), this, SLOT (pathIndexChanged(int)));
  connect (playPause(), SIGNAL (toggled (bool)), this, SLOT (playPauseToggled(bool)));
  connect (stop(), SIGNAL (clicked()), this, SLOT (stopClicked()));
  connect (record(), SIGNAL (toggled(bool)), this, SLOT (recordToggled(bool)));
  connect (main_->solver(), SIGNAL (problemSolved ()), this, SLOT (update()));
}

void PathPlayer::update ()
{
  CORBA::Short nbPath = main_->hppClient ()->problem ()->numberPaths ();
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
  pathLength_ = main_->hppClient()->problem()->pathLength (i);
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
      main_->osg ()->startCapture(
            main_->centralWidget()->windowID(),
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
          currentParam_ == pathLength_;
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
      main_->hppClient()->problem()->configAtParam (pathIndex()->value(),currentParam_);
  main_->hppClient()->robot()->setCurrentConfig (config.in());
  main_->applyCurrentConfiguration();
}

inline double PathPlayer::sliderToLength(int v) const
{
  return ((double)v / (double)pathSlider()->maximum()) * pathLength_;
}

int PathPlayer::lengthToSlider(double l) const
{
  return pathSlider()->maximum() * l / pathLength_;
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
  return findChild <QDoubleSpinBox*> ("timeSpinBox");
}

QSpinBox *PathPlayer::pathIndex() const
{
  return findChild <QSpinBox*> ("pathIndexSpin");
}

QSlider *PathPlayer::pathSlider() const
{
  return findChild <QSlider*> ("pathSlider");
}

QPushButton *PathPlayer::playPause() const
{
  return findChild <QPushButton*> ("playPauseButton");
}

QPushButton *PathPlayer::stop() const
{
  return findChild <QPushButton*> ("stopButton");
}

QPushButton *PathPlayer::record() const
{
  return findChild <QPushButton*> ("recordButton");
}
