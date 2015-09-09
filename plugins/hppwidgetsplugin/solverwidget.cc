#include "hppwidgetsplugin/solverwidget.h"
#include "hppwidgetsplugin/ui_solverwidget.h"

#include <hpp/corbaserver/client.hh>

#include <QFormLayout>
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
  solveDoneId_ (-1)
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

void SolverWidget::displayRoadmap(const std::string jointName)
{
  std::string rn = "roadmap_" + jointName;
  float colorN[] = {1.f, 0.f, 0.f, 1.f};
  float colorE[] = {0.f, 1.f, 0.f, 1.f};
  WindowsManagerPtr_t wsm = MainWindow::instance()->osg();
  HppWidgetsPlugin::HppClient* hpp = plugin_->client();
  int nbNodes = hpp->problem()->numberNodes();
  if (nbNodes == 0) {
      MainWindow::instance()->logError("There is no node in the roadmap.");
      return;
    }
  wsm->createScene (rn.c_str());
  hpp::floatSeq_var curCfg = hpp->robot()->getCurrentConfig();
  for (int i = 0; i < nbNodes; ++i) {
      float pos[7];
      hpp::floatSeq_var n = hpp->problem()->node(i);
      hpp->robot()->setCurrentConfig(n.in());
      hpp::Transform__var t = hpp->robot()->getLinkPosition(jointName.c_str());
      for (int j = 0; j < 7; ++j) { pos[j] = (float)t.in()[j]; }
      QString xyzName = QString::fromStdString(rn).append("/node%1").arg (i);
      wsm->addXYZaxis(xyzName.toLocal8Bit().data(), colorN, 0.01f, 1.f);
      wsm->applyConfiguration(xyzName.toLocal8Bit().data(), pos);
    }
  int nbEdges = hpp->problem()->numberEdges();
  for (int i = 0; i < nbEdges; ++i) {
      hpp::floatSeq_var n1, n2;
      hpp::Transform__var t;
      hpp->problem()->edge(i, n1.out(), n2.out());
      float pos1[3], pos2[3];
      hpp->robot()->setCurrentConfig(n1.in());
      t = hpp->robot()->getLinkPosition(jointName.c_str());
      for (int j = 0; j < 3; ++j) { pos1[j] = (float)t.in()[j]; }
      hpp->robot()->setCurrentConfig(n2.in());
      t = hpp->robot()->getLinkPosition(jointName.c_str());
      for (int j = 0; j < 3; ++j) { pos2[j] = (float)t.in()[j]; }
      QString lineName = QString::fromStdString(rn).append("/edge%1").arg (i);
      wsm->addLine(lineName.toLocal8Bit().data(), pos1, pos2, colorE);
    }
  hpp->robot()->setCurrentConfig(curCfg.in());
  wsm->addToGroup(rn.c_str(), "hpp-gui");
  wsm->refresh();
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

void SolverWidget::interrupt()
{
  plugin_->client()->problem()->interruptPathPlanning();
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

void SolverWidget::selectButtonSolve(bool solve)
{
  if (solve) {
      ui_->pushButtonInterrupt->setVisible(false);
      ui_->pushButtonSolve->setVisible(true);
    } else {
      ui_->pushButtonInterrupt->setVisible(true);
      ui_->pushButtonSolve->setVisible(false);
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
