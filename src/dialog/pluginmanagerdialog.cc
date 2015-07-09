#include "hpp/gui/dialog/pluginmanagerdialog.h"
#include "ui_pluginmanagerdialog.h"

#include "hpp/gui/plugin-interface.h"

bool PluginManager::add(const QString &name, QWidget *parent, bool load) {
  plugins_[name] = new QPluginLoader (name, parent);
  if (load) return loadPlugin(name);
  return false;
}

QIcon PluginManager::icon(const QPluginLoader *pl)
{
  if (pl->isLoaded()) {
      const PluginInterface* pi = const_instance_cast <PluginInterface> (pl);
      if (pi) {
          return QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
        }
    }
  return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
}

QString PluginManager::status(const QPluginLoader *pl)
{
  if (pl->isLoaded()) {
    const PluginInterface* pi = const_instance_cast <PluginInterface> (pl);
    if (pi)
      return QString ("Plugin loaded correctly");
    else
      return QString ("Wrong interface");
  } else
    return pl->errorString();
}

bool PluginManager::loadPlugin(const QString &name) {
  if (!plugins_[name]->load()) {
      qDebug() << name << ": " << plugins_[name]->errorString();
      return false;
    }
  PluginInterface* pi = qobject_cast <PluginInterface*> (plugins_[name]->instance());
  if (!pi) {
      qDebug() << name << ": Wrong interface.";
      return false;
    }
  pi->init();
  return true;
}

PluginManagerDialog::PluginManagerDialog(PluginManager *pm, QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::PluginManagerDialog),
  pm_ (pm)
{
  ui_->setupUi(this);

  for (PluginManager::Map::const_iterator p = pm_->plugins ().constBegin();
       p != pm_->plugins().constEnd(); p++) {
      QString name = p.key(),
          filename = p.value()->fileName(),
          version = "";
      QIcon icon = pm_->icon (p.value());

      ui_->pluginList->insertRow(ui_->pluginList->rowCount());
      ui_->pluginList->setItem(ui_->pluginList->rowCount() - 1, 0, new QTableWidgetItem (icon, name));
      ui_->pluginList->setItem(ui_->pluginList->rowCount() - 1, 1, new QTableWidgetItem (filename));
      ui_->pluginList->setItem(ui_->pluginList->rowCount() - 1, 2, new QTableWidgetItem (version));
    }

  connect(ui_->pluginList, SIGNAL (currentItemChanged (QTableWidgetItem*,QTableWidgetItem*)),
          this, SLOT (onItemChanged(QTableWidgetItem*,QTableWidgetItem*)));
}

PluginManagerDialog::~PluginManagerDialog()
{
  delete ui_;
}

void PluginManagerDialog::onItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
  QString key = ui_->pluginList->item(current->row(), 0)->text();
  const QPluginLoader* pl = pm_->plugins()[key];
  ui_->pluginMessage->setText(pm_->status (pl));
}
