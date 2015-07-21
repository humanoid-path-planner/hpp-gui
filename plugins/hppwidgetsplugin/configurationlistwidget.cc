#include "configurationlistwidget.h"
#include "ui_configurationlistwidget.h"

#include "hpp/corbaserver/common.hh"
#include "hpp/corbaserver/client.hh"

const int ConfigurationListWidget::ConfigRole = Qt::UserRole + 1;

ConfigurationListWidget::ConfigurationListWidget(HppWidgetsPlugin *plugin, QWidget* parent) :
  QWidget (parent),
  plugin_ (plugin),
  ui_ (new Ui::ConfigurationListWidget),
  main_ (MainWindow::instance()),
  basename_ ("config_"), count_ (0)
{
  ui_->setupUi (this);

  connect (ui_->button_SaveConfig, SIGNAL (clicked()), this, SLOT (onSaveClicked()));
  connect (list (), SIGNAL (itemPressed (QListWidgetItem*)),
           this, SLOT (updateCurrentConfig(QListWidgetItem*)));
  connect (list (), SIGNAL (customContextMenuRequested(QPoint)),
           this, SLOT (showListContextMenu (QPoint)));
}

ConfigurationListWidget::~ConfigurationListWidget()
{
  delete ui_;
}

QListWidget *ConfigurationListWidget::list() {
  return ui_->listConfigurations;
}

void ConfigurationListWidget::onSaveClicked ()
{
  hpp::floatSeq* c = plugin_->client()->robot()->getCurrentConfig ();
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
  plugin_->client()->robot()->setCurrentConfig (c);
  emit main_->applyCurrentConfiguration();
}

void ConfigurationListWidget::showListContextMenu (const QPoint& pos)
{
  qDebug () << "Allo";
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
      plugin_->client()->problem()->setInitialConfig (c);
    } else if (selected == goal) {
      plugin_->client()->problem()->addGoalConfig (c);
    } else if (selected == reset) {
      plugin_->client()->problem()->resetGoalConfigs ();
    }
}

QLineEdit *ConfigurationListWidget::name() {
  return ui_->lineEdit_configName;
}
