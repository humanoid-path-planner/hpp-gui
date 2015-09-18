#ifndef SOLVERWIDGET_H
#define SOLVERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QFormLayout>

#include "hpp/gui/fwd.h"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

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

protected slots:
  void selectPathPlanner (const QString& text);
  void selectPathOptimizer (const QString& text);
  void selectPathProjector (int index);
  void solve ();
  void solveAndDisplay ();
  void solveAndDisplayDone ();
  void interrupt ();
  void loadRoadmap ();
  void saveRoadmap ();

  void handleWorkerDone (int id);

private:
  class SolveAndDisplay {
  public:
    bool interrupt;
    bool isSolved;
    QFuture <void> status;
    QFutureWatcher <void> watcher;
    HppWidgetsPlugin* plugin;
    SolverWidget* parent;
    void solve ();
    SolveAndDisplay (HppWidgetsPlugin* p, SolverWidget* par) :
      interrupt (false), isSolved (false),
      plugin (p), parent (par)
    {}
  };

  void selectButtonSolve (bool solve);
  QComboBox* planner ();
  QComboBox* projector ();
  QComboBox* optimizer ();

  Ui::SolverWidget* ui_;
  HppWidgetsPlugin* plugin_;
  MainWindow* main_;

  QComboBox *planner_, *projector_, *optimizer_;

  int solveDoneId_;
  SolveAndDisplay solveAndDisplay_;
};

#endif // SOLVERWIDGET_H
