#include "hpp/gui/dialog/pluginmanagerdialog.h"
#include "ui_pluginmanagerdialog.h"

#include "hpp/gui/plugin-interface.h"

bool PluginManager::add(const QString &name, QWidget *parent, bool load) {
  plugins_[name] = new QPluginLoader (name, parent);
  if (load) return loadPlugin(name);
  return false;
}

bool PluginManager::loadPlugin(const QString &name) {
  if (!plugins_[name]->load()) {
      qDebug() << name << ": " << plugins_[name]->errorString();
      return false;
    }
  PluginInterface* pi = qobject_cast <PluginInterface*> (plugins_[name]->instance());
  if (!pi) {
      qDebug() << name << ": " << plugins_[name]->errorString();
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
      QString text;
      QIcon icon;
      if (p.value()->isLoaded()) {
          text = p.value()->fileName();
          icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
        } else {
          if (p.value()->fileName ().isEmpty ())
            text = p.key();
          else
            text = p.value()->fileName();
          icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        }
      QListWidgetItem* item = new QListWidgetItem (icon, text);
      item->setData(Qt::UserRole, p.key());
      ui_->pluginList->addItem(item);
    }

  connect(ui_->pluginList, SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (onItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

PluginManagerDialog::~PluginManagerDialog()
{
  delete ui_;
}

void PluginManagerDialog::onItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
  QString key = current->data(Qt::UserRole).toString();
  const QPluginLoader* pl = pm_->plugins()[key];
  if (pl->isLoaded())
    ui_->pluginMessage->setText("Plugin loaded correctly");
  else
    ui_->pluginMessage->setText(pl->errorString());
  return;
}
