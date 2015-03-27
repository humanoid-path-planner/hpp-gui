#include "hpp/gui/configurationlistwidget.h"

#include "hpp/corbaserver/common.hh"
#include "hpp/corbaserver/client.hh"

const int ConfigurationListWidget::ConfigRole = Qt::UserRole + 1;

ConfigurationListWidget::ConfigurationListWidget(QWidget* parent) :
  QWidget (parent), main_ (MainWindow::instance())
, basename_ ("config_"), count_ (0)
{
}

ConfigurationListWidget::~ConfigurationListWidget()
{}

void ConfigurationListWidget::onSaveClicked ()
{
  hpp::floatSeq* c = main_->hppClient()->robot()->getCurrentConfig ();
  QVariant var;
  var.setValue (c);
  QLineEdit* n = name();
  QListWidgetItem* newItem = new QListWidgetItem (n->text());
  newItem->setData(ConfigRole, var);
  list()->addItem(newItem);
  n->setText(basename_ + QString::number(count_));
  count_++;
}

void ConfigurationListWidget::updateCurrentConfig (QListWidgetItem* item)
{
  const hpp::floatSeq& c = *(item->data(ConfigRole).value <hpp::floatSeq*> ());
  main_->hppClient()->robot()->setCurrentConfig (c);
  emit main_->applyCurrentConfiguration();
}

void ConfigurationListWidget::showListContextMenu (const QPoint& pos)
{
  QListWidgetItem* item = list()->itemAt(pos);
  if (!item) return;
  QMenu contextMenu(tr("Configuration"), this);
  QAction* init = new QAction(tr("Set as initial configuration"), this);
  QAction* goal = new QAction(tr("Add as goal configuration"), this);
  QAction* reset = new QAction(tr("Reset goal configurations"), this);
  contextMenu.addAction(init);
  contextMenu.addAction(goal);
  contextMenu.addSeparator();
  contextMenu.addAction(reset);
  QAction* selected = contextMenu.exec(mapToGlobal(pos));
  const hpp::floatSeq& c = *(item->data(ConfigRole).value <hpp::floatSeq*> ());
  if (selected == init) {
      main_->hppClient()->problem()->setInitialConfig (c);
    } else if (selected == goal) {
      main_->hppClient()->problem()->addGoalConfig (c);
    } else if (selected == reset) {
      main_->hppClient()->problem()->resetGoalConfigs ();
    }
}
