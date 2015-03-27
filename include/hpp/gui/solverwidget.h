#ifndef SOLVERWIDGET_H
#define SOLVERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QFormLayout>

#include "hpp/gui/fwd.h"

class SolverWidget : public QWidget
{
  Q_OBJECT

public:
  enum Select {
    Planner,
    Optimizer,
    Projector,
    All
  };

  SolverWidget (QWidget* parent = 0);

  ~SolverWidget ();

  virtual void update (Select s = All);

  void setup ();

signals:
  void problemSolved ();

protected slots:
  void selectPathPlanner (const QString& text);
  void selectPathOptimizer (const QString& text);
  void selectPathProjector (int index);
  void solve ();

  void handleWorkerDone (int id);

private:
  QComboBox* planner ();
  QComboBox* projector ();
  QComboBox* optimizer ();

  MainWindow* main_;

  QComboBox *planner_, *projector_, *optimizer_;

  int solveDoneId_;
};

#endif // SOLVERWIDGET_H
