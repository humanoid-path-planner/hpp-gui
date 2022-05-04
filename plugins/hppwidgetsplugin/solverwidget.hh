//
// Copyright (c) CNRS
// Author: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_SOLVERWIDGET_HH
#define HPP_GUI_SOLVERWIDGET_HH

#include <QWidget>

#include "gepetto/gui/fwd.hh"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

class QComboBox;
class QDoubleSpinBox;

namespace Ui {
class SolverWidget;
}

namespace hpp {
namespace gui {
class SolverWidget : public QWidget {
  Q_OBJECT

 public:
  enum Select {
    Planner,
    Optimizer,
    Projector,
    Validation,
    SteeringMethod,
    All
  };

  SolverWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);

  ~SolverWidget();

 public slots:
  /// Update the field in the widget.
  /// \param s which field to update
  virtual void update(Select s = All);

 signals:
  void problemSolved();

 protected slots:
  /// Change the path planner use in hpp.
  /// \param text name of the path planner
  void selectPathPlanner(const QString& text);

  /// Change the path Optimizers in hpp.
  /// \param list list of path optimizers
  void selectPathOptimizers(const QStringList& list);
  void openPathOptimizerSelector();

  /// Change the path projector in hpp.
  /// \param name name of the path projector
  void selectPathProjector(const QString& name);

  /// Change the path validation in hpp.
  /// \param name name of the path validation
  void selectPathValidation(const QString& name);

  /// Change the steering method in hpp.
  /// \param name name of the path projector
  void selectSteeringMethod(const QString& name);

  /// Change the allowed discontinuity in hpp
  /// \param value new value of discontinuity
  void discontinuityChanged(double value);

  /// Change the allowed penetration in hpp
  /// \param value new value of penetration
  void penetrationChanged(double value);

  /// Ask hpp to solve the problem.
  void solve();

  /// Ask hpp to solve the problem and display the roadmap of the joint
  /// selected in the joint tree.
  void solveAndDisplay();

  /// Interrupt the solver.
  void interrupt();

  /// Load a roadmap in hpp.
  void loadRoadmap();

  /// Save the roadmap from hpp.
  void saveRoadmap();

  /// Reset the roadmap in hpp.
  void clearRoadmap();

  /// Optimize the path currently selected in PathPlayer widget.
  void optimizePath();

  void solveDone();

 private:
  struct Solve {
    bool interrupt, stepByStep, isSolved;
    QFuture<void> status;
    QFutureWatcher<void> watcher;
    HppWidgetsPlugin* plugin;
    SolverWidget* parent;
    void solve();
    void optimize(const int pathId);
    void solveAndDisplay();
    bool done();
    Solve(HppWidgetsPlugin* p, SolverWidget* par)
        : interrupt(false),
          stepByStep(false),
          isSolved(false),
          plugin(p),
          parent(par) {}
  };

  void setSelected(QComboBox* cb, QString const& what);
  void selectButtonSolve(bool solve);
  QComboBox* planner();
  QComboBox* projector();
  QComboBox* validation();
  QComboBox* steeringMethod();
  QDoubleSpinBox* projectorDiscontinuity();
  QDoubleSpinBox* validationPenetration();

  ::Ui::SolverWidget* ui_;
  HppWidgetsPlugin* plugin_;
  gepetto::gui::MainWindow* main_;

  QStringList optimizers_;

  Solve solve_;
  friend struct Solve;
};
}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_SOLVERWIDGET_HH
