#include "jointtreewidget.h"
#include "ui_jointtreewidget.h"

#include <hpp/gui/mainwindow.h>

#include "joint-tree-item.h"

JointTreeWidget::JointTreeWidget(HppWidgetsPlugin *plugin, QWidget *parent) :
  QWidget(parent),
  plugin_ (plugin),
  ui_ (new Ui::JointTreeWidget),
  model_ (new QStandardItemModel),
  dock_ (NULL)
{
  ui_->setupUi (this);
  ui_->jointTree->setModel(model_);
  ui_->jointTree->setItemDelegate (
        new JointItemDelegate(ui_->button_forceVelocity,
                              plugin_,
                              MainWindow::instance()));
  reset ();

  connect(ui_->jointTree, SIGNAL (customContextMenuRequested(QPoint)),
          this, SLOT (customContextMenu(QPoint)));
  connect (refreshButton (), SIGNAL (clicked ()),
           this, SLOT (reload()));
}

JointTreeWidget::~JointTreeWidget()
{
  delete ui_;
}

const QPushButton *JointTreeWidget::refreshButton() const
{
  return ui_->refreshButton;
}

void JointTreeWidget::dockWidget(QDockWidget *dock)
{
  dock_ = dock;
}

void JointTreeWidget::customContextMenu(const QPoint &pos)
{
  QModelIndex index = ui_->jointTree->indexAt(pos);
  if(index.isValid()) {
      QMenu contextMenu (tr("Node"), this);
      JointModifierInterface* adi =
          MainWindow::instance()->pluginManager ()->getFirstOf <JointModifierInterface> ();
      if (!adi) return;
      JointTreeItem *item =
          dynamic_cast <JointTreeItem*> (model_->itemFromIndex(index));
      if (!item) return;
      contextMenu.addAction (adi->action (item->name()));
      contextMenu.exec(ui_->jointTree->mapToGlobal(pos));
      return;
    }
}

void JointTreeWidget::addJointToTree(const std::string name, JointTreeItem *parent)
{
  MainWindow* main = MainWindow::instance();
  HppWidgetsPlugin::JointElement& je = plugin_->jointMap() [name];
  graphics::NodePtr_t node = main->osg ()->getNode(je.bodyName);
  if (!node) node = main->osg ()->getScene(je.bodyName);
  hpp::floatSeq_var c = plugin_->client()->robot ()->getJointConfig (name.c_str());
  CORBA::Short nbDof = plugin_->client()->robot ()->getJointNumberDof (name.c_str());
  hpp::corbaserver::jointBoundSeq_var b = plugin_->client()->robot ()->getJointBounds (name.c_str());

  JointTreeItem* j = new JointTreeItem (name.c_str(), c.in(), b.in(), nbDof, node);
  je.item = j;
  if (parent) parent->appendRow(j);
  else        model_->appendRow(j);
  hpp::Names_t_var children = plugin_->client()->robot ()->getChildJointNames (name.c_str());
  for (size_t i = 0; i < children->length(); ++i)
    addJointToTree(std::string(children[i]),j);
}

void JointTreeWidget::selectJoint(const std::string &jointName)
{
  HppWidgetsPlugin::JointMap::const_iterator itj = plugin_->jointMap().find(jointName);
  if (itj == plugin_->jointMap().constEnd()) return;
  const HppWidgetsPlugin::JointElement& je = itj.value();
  if (!je.item) return;
  qDebug () << "Selected joint: " << QString::fromStdString(je.name);
  ui_->jointTree->clearSelection();
  ui_->jointTree->setCurrentIndex(je.item->index());
  if (dock_ != NULL && !dock_->isVisible())
      dock_->setVisible(true);
}

void JointTreeWidget::reload()
{
  reset ();
  plugin_->jointMap ().clear();
  char* robotName;
  try {
    robotName = plugin_->client ()->robot()->getRobotName();
  } catch (hpp::Error& e) {
    MainWindow::instance ()->logError(QString(e.msg));
    return;
  }
  hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames ();
  std::string bjn (joints[0]);
  plugin_->updateRobotJoints(robotName);
  addJointToTree(bjn, 0);
  delete[] robotName;
}

void JointTreeWidget::reset()
{
  model_->clear();
  ui_->jointTree->header()->setVisible(true);
  QStringList l; l << "Joint" << "Lower bound" << "Upper bound";
  model_->setHorizontalHeaderLabels(l);
  model_->setColumnCount(3);
}
