#ifndef PATHPLAYER_H
#define PATHPLAYER_H

#include <QWidget>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>

#include <hpp/gui/fwd.h>

class PathPlayer : public QWidget
{
  Q_OBJECT

public:
  PathPlayer (QWidget* parent = 0);
  ~PathPlayer();

  void setup ();

private slots:
  void update ();

  void pathIndexChanged (int i);
  void pathSliderChanged (int value);
//  void timeChanged (double d);
  void playPauseToggled (bool toggled);
  void stopClicked ();
  void recordToggled (bool toggled);
  void pathPulse ();
  void timerEvent(QTimerEvent *event);

private:
  void updateConfiguration ();
  double sliderToLength (int v) const;
  int lengthToSlider (double l) const;
  double timeToLength (double time) const;
  int timeBetweenRefresh() const;
  double lengthBetweenRefresh () const;

  QDoubleSpinBox* timeSpinBox () const;
  QSpinBox* pathIndex() const;
  QSlider* pathSlider () const;
  QPushButton* playPause () const;
  QPushButton* stop () const;
  QPushButton* record () const;

  MainWindow* main_;

  const int frameRate_;
  double pathLength_;
  double currentParam_;
  int timerId_;
};

#endif // PATHPLAYER_H
