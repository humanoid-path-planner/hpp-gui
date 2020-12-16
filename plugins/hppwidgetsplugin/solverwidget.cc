//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include "hppwidgetsplugin/solverwidget.hh"
#include "hppwidgetsplugin/ui_solverwidget.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QListWidget>
#include <QMessageBox>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
# include <QtCore>
#else
# include <QtConcurrent>
#endif

#include <hpp/corbaserver/client.hh>

#include "gepetto/gui/mainwindow.hh"
#include "gepetto/gui/windows-manager.hh"

#include "hppwidgetsplugin/roadmap.hh"
#include "hppwidgetsplugin/pathplayer.hh"

namespace hpp {
  namespace gui {
    namespace {
      void clearQComboBox (QComboBox* c) {
        while (c->count() > 0) c->removeItem(0);
      }
    }

    using gepetto::gui::MainWindow;

    SolverWidget::SolverWidget (HppWidgetsPlugin *plugin, QWidget *parent) :
      QWidget (parent),
      ui_ (new ::Ui::SolverWidget),
      plugin_ (plugin),
      main_(MainWindow::instance()),
      solve_ (plugin, this)
    {
      ui_->setupUi (this);
      selectButtonSolve(true);

      connect(planner(), SIGNAL (activated(const QString&)), this, SLOT (selectPathPlanner(const QString&)));
      connect(ui_->pathOptimizerButton, SIGNAL (clicked()), this, SLOT (openPathOptimizerSelector()));
      connect(projector(), SIGNAL (activated(const QString&)), this, SLOT (selectPathProjector(const QString&)));
      connect(validation(), SIGNAL (activated(const QString&)), this, SLOT (selectPathValidation(const QString&)));
      connect(steeringMethod(), SIGNAL(activated(const QString&)), this, SLOT(selectSteeringMethod(const QString&)));
      connect(ui_->pushButtonSolve, SIGNAL (clicked ()), this, SLOT (solve ()));
      connect(ui_->pushButtonInterrupt, SIGNAL (clicked ()), this, SLOT (interrupt ()));
      connect(ui_->pushButtonSolveAndDisplay, SIGNAL (clicked ()),
          SLOT (solveAndDisplay ()));
      connect(&solve_.watcher, SIGNAL (finished()), SLOT(solveDone ()));
      connect(ui_->loadRoadmap, SIGNAL (clicked()), SLOT (loadRoadmap()));
      connect(ui_->saveRoadmap, SIGNAL (clicked()), SLOT (saveRoadmap()));
      connect(ui_->clearRoadmap, SIGNAL (clicked()), SLOT (clearRoadmap()));
      connect(ui_->optimizeButton, SIGNAL(clicked()), SLOT(optimizePath()));

      // Settings of the DoubleSpinBox for discontinuity
      ui_->pathProjectorDiscontinuity->setMinimum(0);
      ui_->pathProjectorDiscontinuity->setValue(0.2);
      ui_->pathProjectorDiscontinuity->setSingleStep(0.1);
      connect(ui_->pathProjectorDiscontinuity, SIGNAL(valueChanged(double)),
	      this, SLOT(discontinuityChanged(double)));

      // Settings of the DoubleSpinBox for penetration
      ui_->pathValidationPenetration->setMinimum(0);
      ui_->pathValidationPenetration->setValue(0.05);
      ui_->pathValidationPenetration->setSingleStep(0.01);
      connect(ui_->pathValidationPenetration, SIGNAL(valueChanged(double)),
	      this, SLOT(penetrationChanged(double)));
    }

    SolverWidget::~SolverWidget()
    {
      delete ui_;
    }

    void SolverWidget::setSelected(QComboBox *cb, const QString &what)
    {
      hpp::Names_t_var names = plugin_->client()->problem()->getSelected(what.toStdString().c_str());
      cb->setCurrentIndex(cb->findText(names[0].in()));
    }

    void SolverWidget::update (Select s) {
      hpp::Names_t_var names;
      switch (s) {
        case All:
        case Planner:
          clearQComboBox(planner());
          names = plugin_->client()->problem()->getAvailable("PathPlanner");
          for (CORBA::ULong i = 0; i < names->length(); ++i)
            planner()->addItem(QString::fromLocal8Bit(names[i]));
          setSelected(planner(), "PathPlanner");
          if (s == Planner) break;
          // fall through
        case Optimizer:
          names = plugin_->client()->problem()->getAvailable("PathOptimizer");
          optimizers_.clear();
          for (CORBA::ULong i = 0; i < names->length(); ++i)
            optimizers_ << QString::fromLocal8Bit(names[i]);
          if (s == Optimizer) break;
          // fall through
        case Validation:
	  clearQComboBox(validation());
          names = plugin_->client()->problem()->getAvailable("PathValidation");
          for (CORBA::ULong i = 0; i < names->length(); ++i)
            validation()->addItem(QString::fromLocal8Bit(names[i]), QVariant (0.2));
          setSelected(validation(), "PathValidation");
          if (s == Validation) break;
          // fall through
        case Projector:
          clearQComboBox(projector());
          names = plugin_->client()->problem()->getAvailable("PathProjector");
          for (CORBA::ULong i = 0; i < names->length(); ++i)
            projector()->addItem(QString::fromLocal8Bit(names[i]), QVariant (0.2));
          setSelected(projector(), "PathProjector");
          if (s == Projector) break;
          // fall through
        case SteeringMethod:
          clearQComboBox(steeringMethod());
          names = plugin_->client()->problem()->getAvailable("SteeringMethod");
          for (CORBA::ULong i = 0; i < names->length(); ++i)
            steeringMethod()->addItem(QString::fromLocal8Bit(names[i]));
          setSelected(steeringMethod(), "SteeringMethod");
          if (s == SteeringMethod) break;
          // fall through
      }
    }

    void SolverWidget::selectPathPlanner (const QString& text) {
      plugin_->client()->problem()->selectPathPlanner (text.toStdString().c_str());
    }

    void SolverWidget::selectSteeringMethod (const QString& text) {
      plugin_->client()->problem()->selectSteeringMethod (text.toStdString().c_str());
    }

    void SolverWidget::selectPathOptimizers (const QStringList& list) {
      plugin_->client()->problem()->clearPathOptimizers();
      foreach(QString s, list)
        plugin_->client()->problem()->addPathOptimizer (s.toStdString().c_str());
    }

    void SolverWidget::selectPathProjector (const QString& name) {
      plugin_->client()->problem()->selectPathProjector (
          name.toStdString().c_str(),
	  projectorDiscontinuity()->value());
    }

    void SolverWidget::selectPathValidation (const QString& name) {
      plugin_->client()->problem()->selectPathValidation (
      name.toStdString().c_str(),
          validationPenetration()->value());
    }

    void SolverWidget::discontinuityChanged(double value)
    {
      Q_UNUSED(value);
      selectPathProjector(projector()->currentText());
    }

    void SolverWidget::penetrationChanged(double value)
    {
      Q_UNUSED(value);
      selectPathValidation(validation()->currentText());
    }

    void SolverWidget::openPathOptimizerSelector ()
    {
      QDialog dialog(this);
      // Use a layout allowing to have a label next to each field
      QVBoxLayout lay(&dialog);

      // Add some text above the fields
      lay.addWidget(new QLabel("Select path optimizers:"));

      // Add the lineEdits with their respective labels
      QListWidget* list = new QListWidget (&dialog);
      list->addItems(optimizers_);
      list->setSelectionMode(QAbstractItemView::ExtendedSelection);
      lay.addWidget(list);

      hpp::Names_t_var names = plugin_->client()->problem()->getSelected("PathOptimizer");
      for (unsigned i = 0; i < names->length(); ++i) {
        int index = optimizers_.indexOf(QRegExp(names[i].in()));

        list->item(index)->setSelected(true);
      }

      QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
          Qt::Horizontal, &dialog);
      lay.addWidget(&buttonBox);
      QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
      QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

      // Show the dialog as modal
      if (dialog.exec() == QDialog::Accepted) {
          // If the user didn't dismiss the dialog, do something with the fields
          QStringList selected;
          foreach (QListWidgetItem* item, list->selectedItems())
              selected << item->text();
          qDebug () << "Selected path optimizers: " << selected;
          selectPathOptimizers(selected);
        }
    }

    void SolverWidget::solve()
    {
      if (solve_.status.isRunning ()) {
        main_->logError("The solver is already running.");
        return;
      }
      solve_.stepByStep = false;
      solve_.status = QtConcurrent::run(&solve_, &Solve::solve);
      solve_.watcher.setFuture (solve_.status);
      main_->logJobStarted(0, "solve problem.");
      selectButtonSolve(false);
    }

    void SolverWidget::solveAndDisplay()
    {
      if (solve_.status.isRunning ()) {
        main_->logError("The solver is already running.");
        return;
      }
      solve_.stepByStep = true;
      solve_.interrupt = false;
      solve_.status = QtConcurrent::run (&solve_, &Solve::solveAndDisplay);
      solve_.watcher.setFuture (solve_.status);
      selectButtonSolve (false);
    }

    void SolverWidget::solveDone()
    {
      if (solve_.done())
        emit problemSolved ();
    }

    bool SolverWidget::Solve::done()
    {
      qDebug () << "Solve done";
      parent->selectButtonSolve (true);
      if (isSolved) {
        QMessageBox::information(parent, "Problem solver", "Problem is solved.");
        parent->main_->logJobDone(0, "Problem solved.");
        return true;
      }
      return false;
    }

    void SolverWidget::interrupt()
    {
      if (solve_.stepByStep) {
        solve_.interrupt = true;
      } else {
        plugin_->client()->problem()->interruptPathPlanning();
      }
      solve_.status.waitForFinished ();
      selectButtonSolve(true);
    }

    void SolverWidget::loadRoadmap()
    {
      QString file = QFileDialog::getOpenFileName(this, tr ("Select a roadmap file"));
      if (file.isNull()) return;
      try {
        plugin_->client()->problem()->loadRoadmap (file.toLocal8Bit().data());
      } catch (const hpp::Error& e) {
        MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
      }
    }

    void SolverWidget::saveRoadmap()
    {
      QString file = QFileDialog::getSaveFileName(this, tr("Select a destination"));
      if (file.isNull()) return;
      try {
        plugin_->client()->problem()->saveRoadmap (file.toLocal8Bit().data());
      } catch (const hpp::Error& e) {
        MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
      }
    }

    void SolverWidget::clearRoadmap()
    {
      try {
        plugin_->client()->problem()->resetRoadmap ();
      } catch (const hpp::Error& e) {
        MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
      }
    }

    void SolverWidget::optimizePath()
    {
      if (solve_.status.isRunning ()) {
        main_->logError("The solver is already running.");
        return;
      }
      solve_.stepByStep = false;
      solve_.status = QtConcurrent::run(&solve_,
          &Solve::optimize, plugin_->pathPlayer()->getCurrentPath());
      solve_.watcher.setFuture (solve_.status);
      main_->logJobStarted(0, "Optimize path.");
      selectButtonSolve(false);
    }

    void SolverWidget::Solve::solve()
    {
      isSolved = false;
      HppWidgetsPlugin::HppClient* hpp = plugin->client();
      try {
        hpp::intSeq_var time = hpp->problem()->solve();
        isSolved = true;
      } catch (hpp::Error const& e) {
        parent->main_->logJobFailed(0, QString(e.msg));
      }
    }

    void SolverWidget::Solve::optimize(const int pathId)
    {
      isSolved = false;
      HppWidgetsPlugin::HppClient* hpp = plugin->client();
      try {
        hpp->problem()->optimizePath((CORBA::UShort)pathId);
        isSolved = true;
      } catch (hpp::Error const& e) {
        parent->main_->logJobFailed(0, QString(e.msg));
      }
    }

    void SolverWidget::Solve::solveAndDisplay()
    {
      isSolved = false;
      HppWidgetsPlugin::HppClient* hpp = plugin->client();
      std::string jn = plugin->getSelectedJoint();
      if (jn.empty()) {
	QMessageBox::information(parent, "Problem solver",
				 "Please, select a joint in the joint tree window.");
        return;
      }
      isSolved = hpp->problem()->prepareSolveStepByStep();
      Roadmap* r = plugin->createRoadmap(jn);
      while (!isSolved) {
        isSolved = hpp->problem()->executeOneStep();
        r->displayRoadmap();
        if (interrupt) break;
      }
      if (isSolved)
        hpp->problem()->finishSolveStepByStep();
      delete r;
    }

    void SolverWidget::selectButtonSolve(bool solve)
    {
      if (solve) {
        ui_->pushButtonInterrupt->setVisible(false);
        ui_->pushButtonSolve->setVisible(true);
        ui_->pushButtonSolveAndDisplay->setVisible(true);
	ui_->optimizeButton->setVisible(true);
      } else {
        ui_->pushButtonInterrupt->setVisible(true);
        ui_->pushButtonSolve->setVisible(false);
        ui_->pushButtonSolveAndDisplay->setVisible(false);
	ui_->optimizeButton->setVisible(false);
      }
    }

    QComboBox *SolverWidget::planner()
    {
      return ui_->pathPlannerComboBox;
    }

    QComboBox *SolverWidget::projector()
    {
      return ui_->pathProjectorComboBox;
    }

    QComboBox *SolverWidget::validation()
    {
      return ui_->pathValidationComboBox;
    }

    QComboBox *SolverWidget::steeringMethod()
    {
      return ui_->steeringMethodComboBox;
    }

    QDoubleSpinBox *SolverWidget::projectorDiscontinuity()
    {
      return ui_->pathProjectorDiscontinuity;
    }

    QDoubleSpinBox *SolverWidget::validationPenetration()
    {
      return ui_->pathValidationPenetration;
    }
  } // namespace gui
} // namespace hpp
