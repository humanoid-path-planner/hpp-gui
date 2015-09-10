#include "hppwidgetsplugin/solverwidget.h"
#include "hppwidgetsplugin/ui_solverwidget.h"

#include <hpp/corbaserver/client.hh>

#include <QFormLayout>
#include <QMessageBox>
#include "hpp/gui/mainwindow.h"
#include "hpp/gui/windows-manager.h"

#include "hppwidgetsplugin/roadmap.hh"

namespace {
  void clearQComboBox (QComboBox* c) {
    while (c->count() > 0) c->removeItem(0);
  }
}

SolverWidget::SolverWidget (HppWidgetsPlugin *plugin, QWidget *parent) :
  QWidget (parent),
  ui_ (new Ui::SolverWidget),
  plugin_ (plugin),
  main_(MainWindow::instance()),
  planner_ (0), projector_ (0), optimizer_ (0),
  solveDoneId_ (-1),
  solveAndDisplay_ (plugin, this)
{
  ui_->setupUi (this);
  selectButtonSolve(true);
  connect (&main_->worker(), SIGNAL (done(int)), this, SLOT (handleWorkerDone (int)));

  update ();
  connect(planner(), SIGNAL (currentIndexChanged(const QString&)), this, SLOT (selectPathPlanner(const QString&)));
  connect(optimizer(), SIGNAL (currentIndexChanged(const QString&)), this, SLOT (selectPathOptimizer(const QString&)));
  connect(projector(), SIGNAL (currentIndexChanged(int)), this, SLOT (selectPathProjector(int)));
  connect(ui_->pushButtonSolve, SIGNAL (clicked ()), this, SLOT (solve ()));
  connect(ui_->pushButtonInterrupt, SIGNAL (clicked ()), this, SLOT (interrupt ()));
  connect(ui_->pushButtonSolveAndDisplay, SIGNAL (clicked ()),
          SLOT (solveAndDisplay ()));
  connect(&solveAndDisplay_.watcher, SIGNAL (finished()),
          SLOT(solveAndDisplayDone ()));
  connect(ui_->loadRoadmap, SIGNAL (clicked()), SLOT (loadRoadmap()));
  connect(ui_->saveRoadmap, SIGNAL (clicked()), SLOT (saveRoadmap()));
}

SolverWidget::~SolverWidget()
{
  delete ui_;
}

void SolverWidget::update (Select s) {
  switch (s) {
    case All:
    case Planner:
      clearQComboBox(planner());
      planner()->addItems(QStringList () << "DiffusingPlanner" << "VisibilityPrmPlanner");
      if (s == Planner) break;
    case Optimizer:
      clearQComboBox(optimizer());
      optimizer()->addItems(QStringList () << "None" << "RandomShortcut");
      if (s == Optimizer) break;
    case Projector:
      clearQComboBox(projector());
      projector()->addItem(QString("None"), QVariant ((double)0.2));
      projector()->addItem(QString("Progressive"), QVariant ((double)0.2));
      projector()->addItem(QString("Dichotomy"), QVariant ((double)0.2));
      if (s == Projector) break;
    }
}

void SolverWidget::selectPathPlanner (const QString& text) {
  plugin_->client()->problem()->selectPathPlanner (text.toStdString().c_str());
}

void SolverWidget::selectPathOptimizer (const QString& text) {
  plugin_->client()->problem()->clearPathOptimizers();
  plugin_->client()->problem()->addPathOptimizer (text.toStdString().c_str());
}

void SolverWidget::selectPathProjector (int index) {
  plugin_->client()->problem()->selectPathProjector (
        projector()->itemText(index).toStdString().c_str(),
        projector()->itemData(index).toDouble());
}

void SolverWidget::solve()
{
  /* double time = */
  WorkItem* item = new WorkItem_0 <hpp::corbaserver::_objref_Problem, CORBA::Double>
      (plugin_->client()->problem().in(), &hpp::corbaserver::_objref_Problem::solve);
  main_->emitSendToBackground(item);
  main_->logJobStarted(item->id(), "solve problem.");
  solveDoneId_ = item->id();
  selectButtonSolve(false);
}

void SolverWidget::solveAndDisplay()
{
  solveAndDisplay_.interrupt = false;
  solveAndDisplay_.status = QtConcurrent::run (&solveAndDisplay_, &SolveAndDisplay::solve);
  solveAndDisplay_.watcher.setFuture (solveAndDisplay_.status);
  selectButtonSolve (false);
}

void SolverWidget::solveAndDisplayDone()
{
  qDebug () << "Step by step done";
  selectButtonSolve (true);
  if (solveAndDisplay_.isSolved) {
      emit problemSolved ();
      QMessageBox::information(this, "Problem solver", "Problem is solved.");
    }
}

void SolverWidget::interrupt()
{
  if (solveAndDisplay_.status.isRunning ()) {
      solveAndDisplay_.interrupt = true;
      solveAndDisplay_.status.waitForFinished ();
    } else {
      plugin_->client()->problem()->interruptPathPlanning();
    }
  selectButtonSolve(true);
}

void SolverWidget::loadRoadmap()
{
  QString file = QFileDialog::getOpenFileName(this, tr ("Select a roadmap file"));
  if (file.isNull()) return;
  try {
    plugin_->client()->problem()->readRoadmap (file.toLocal8Bit().data());
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

void SolverWidget::handleWorkerDone(int id)
{
  if (id == solveDoneId_) {
      emit problemSolved();
      selectButtonSolve(true);
      QMessageBox::information(this, "Problem solver", "Problem is solved.");
      main_->logJobDone(id, "Problem solved.");
    }
}

void SolverWidget::SolveAndDisplay::solve()
{
  HppWidgetsPlugin::HppClient* hpp = plugin->client();
  std::string jn = plugin->getSelectedJoint();
  if (jn.empty()) {
      QMessageBox::information(parent, "Select a joint",
                               "Please, select a joint in the joint tree window.");
      return;
    }
  isSolved = hpp->problem()->prepareSolveStepByStep();
  Roadmap* r = plugin->createRoadmap(plugin->getSelectedJoint());
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
    } else {
      ui_->pushButtonInterrupt->setVisible(true);
      ui_->pushButtonSolve->setVisible(false);
      ui_->pushButtonSolveAndDisplay->setVisible(false);
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

QComboBox *SolverWidget::optimizer()
{
  return ui_->pathOptimizerComboBox;
}
