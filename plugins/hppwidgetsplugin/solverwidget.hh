#ifndef HPP_GUI_SOLVERWIDGET_HH
#define HPP_GUI_SOLVERWIDGET_HH

#include <QWidget>
#include <QComboBox>
#include <QFormLayout>

#include "hpp/gui/fwd.hh"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace Ui {
  class SolverWidget;
}

namespace hpp {
  namespace gui {
    class SolverWidget : public QWidget
    {
      Q_OBJECT

      public:
        enum Select {
          Planner,
          Optimizer,
          Projector,
	  Validation,
          All
        };

        SolverWidget (HppWidgetsPlugin* plugin, QWidget* parent = 0);

        ~SolverWidget ();

      public slots:
        virtual void update (Select s = All);

signals:
        void problemSolved ();

        protected slots:
          void selectPathPlanner (const QString& text);
        void selectPathOptimizers (const QStringList& list);
        void openPathOptimizerSelector ();
        void selectPathProjector (int index);
        void selectPathValidation (int index);
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
        QComboBox* validation ();

        ::Ui::SolverWidget* ui_;
        HppWidgetsPlugin* plugin_;
        MainWindow* main_;

        QStringList optimizers_;

        int solveDoneId_;
        SolveAndDisplay solveAndDisplay_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_SOLVERWIDGET_HH
