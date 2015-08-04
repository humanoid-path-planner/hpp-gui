#ifndef SOLVERWIDGET_H
#define SOLVERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QFormLayout>

#include "hpp/gui/fwd.h"
#include "hppwidgetsplugin.hh"

namespace Ui {
  class SolverWidget;
}

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

  SolverWidget (HppWidgetsPlugin* plugin, QWidget* parent = 0);

  ~SolverWidget ();

  virtual void update (Select s = All);

signals:
  void problemSolved ();

public slots:
  void displayRoadmap (const std::string jointName);

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

  Ui::SolverWidget* ui_;
  HppWidgetsPlugin* plugin_;
  MainWindow* main_;

  QComboBox *planner_, *projector_, *optimizer_;

  int solveDoneId_;
};

#endif // SOLVERWIDGET_H
