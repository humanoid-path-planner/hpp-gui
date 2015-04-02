#include "hpp/gui/solverwidget.h"

#include <hpp/corbaserver/client.hh>

#include <QFormLayout>
#include "hpp/gui/mainwindow.h"

namespace {
  void clearQComboBox (QComboBox* c) {
    while (c->count() > 0) c->removeItem(0);
  }
}

SolverWidget::SolverWidget (QWidget *parent) :
  QWidget (parent), main_(MainWindow::instance()),
  planner_ (0), projector_ (0), optimizer_ (0),
  solveDoneId_ (-1)
{
  connect (&main_->worker(), SIGNAL (done(int)), this, SLOT (handleWorkerDone (int)));
}

SolverWidget::~SolverWidget()
{
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
      optimizer()->addItems(QStringList () << "RandomShortcut");
      if (s == Optimizer) break;
    case Projector:
      clearQComboBox(projector());
      projector()->addItem(QString("None"), QVariant ((double)0.2));
      projector()->addItem(QString("Progressive"), QVariant ((double)0.2));
      projector()->addItem(QString("Dichotomy"), QVariant ((double)0.2));
      if (s == Projector) break;
  }
}

void SolverWidget::setup()
{
  update ();
  connect(planner(), SIGNAL (currentIndexChanged(const QString&)), this, SLOT (selectPathPlanner(const QString&)));
  connect(optimizer(), SIGNAL (currentIndexChanged(const QString&)), this, SLOT (selectPathOptimizer(const QString&)));
  connect(projector(), SIGNAL (currentIndexChanged(int)), this, SLOT (selectPathProjector(int)));
  connect(findChild<QPushButton*> ("pushButtonSolve"), SIGNAL (clicked ()), this, SLOT (solve ()));
}

void SolverWidget::selectPathPlanner (const QString& text) {
  main_->hppClient()->problem()->selectPathPlanner (text.toStdString().c_str());
}

void SolverWidget::selectPathOptimizer (const QString& text) {
  main_->hppClient()->problem()->selectPathOptimizer (text.toStdString().c_str());
}

void SolverWidget::selectPathProjector (int index) {
  main_->hppClient()->problem()->selectPathProjector (
        projector()->itemText(index).toStdString().c_str(),
        projector()->itemData(index).toDouble());
}

void SolverWidget::solve()
{
  /* double time = */
  WorkItem* item = new WorkItem_0 <hpp::corbaserver::_objref_Problem, CORBA::Double>
      (main_->hppClient()->problem().in(), &hpp::corbaserver::_objref_Problem::solve);
  main_->emitSendToBackground(item);
  main_->logJobStarted(item->id(), "solve problem.");
  solveDoneId_ = item->id();
}

void SolverWidget::handleWorkerDone(int id)
{
  if (id == solveDoneId_) {
      emit problemSolved();
      main_->logJobDone(id, "Problem solved.");
    }
}

QComboBox *SolverWidget::planner()
{
  if (planner_ == 0) planner_ = findChild <QComboBox*> ("pathPlannerComboBox");
  return planner_;
}

QComboBox *SolverWidget::projector()
{
  if (projector_ == 0) projector_ = findChild <QComboBox*> ("pathProjectorComboBox");
  return projector_;
}

QComboBox *SolverWidget::optimizer()
{
  if (optimizer_ == 0) optimizer_ = findChild <QComboBox*> ("pathOptimizerComboBox");
  return optimizer_;
}