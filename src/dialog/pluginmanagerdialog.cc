#include "hpp/gui/dialog/pluginmanagerdialog.h"
#include "ui_pluginmanagerdialog.h"

#include <QDebug>

#include "hpp/gui/plugin-interface.h"

QList <QDir> PluginManager::pluginDirs_;

bool PluginManager::add(const QString &name, QWidget *parent, bool load) {
  QString filename = name;
  if (!QDir::isAbsolutePath(name)) {
    foreach (QDir dir, pluginDirs_) {
        if (dir.exists(name)) {
            filename = dir.absoluteFilePath(name);
        }
    }
  }
  plugins_[name] = new QPluginLoader (filename, parent);
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

void PluginManager::addPluginDir(const QString &path)
{
    QDir dir (QDir::cleanPath(path));
    QDir can (dir.canonicalPath());
    if (can.exists() && can.isReadable())
        pluginDirs_.append (can);
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

bool PluginManager::unloadPlugin(const QString &name)
{
    return plugins_[name]->unload();
}

PluginManagerDialog::PluginManagerDialog(PluginManager *pm, QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::PluginManagerDialog),
  pm_ (pm)
{
  ui_->setupUi(this);

  updateList ();

  connect(ui_->pluginList, SIGNAL (currentItemChanged (QTableWidgetItem*,QTableWidgetItem*)),
          this, SLOT (onItemChanged(QTableWidgetItem*,QTableWidgetItem*)));
  connect(ui_->pluginList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(contextMenu(QPoint)));
}

PluginManagerDialog::~PluginManagerDialog()
{
  delete ui_;
}

void PluginManagerDialog::onItemChanged(QTableWidgetItem *current,
                                        QTableWidgetItem */*previous*/)
{
  if (!current) return;
  QString key = ui_->pluginList->item(current->row(), 0)->text();
  const QPluginLoader* pl = pm_->plugins()[key];
  ui_->pluginMessage->setText(pm_->status (pl));
}

void PluginManagerDialog::contextMenu(const QPoint &pos)
{
    int row = ui_->pluginList->rowAt(pos.y());
    if (row == -1) return;
    QString key = ui_->pluginList->item(row, 0)->text();
    QMenu contextMenu (tr("Plugin"), ui_->pluginList);
    if (pm_->plugins()[key]->isLoaded()) {
        QAction* unload = contextMenu.addAction("&Unload", &signalMapper_, SLOT(map()));
        signalMapper_.setMapping (unload, key);
        connect(&signalMapper_, SIGNAL (mapped(QString)), this, SLOT(unload(QString)));
        contextMenu.exec(ui_->pluginList->mapToGlobal(pos));
    } else {
        QAction* load = contextMenu.addAction("&Load", &signalMapper_, SLOT(map()));
        signalMapper_.setMapping (load, key);
        connect(&signalMapper_, SIGNAL (mapped(QString)), this, SLOT(load(QString)));
        contextMenu.exec(ui_->pluginList->mapToGlobal(pos));
    }
}

void PluginManagerDialog::load(const QString &name)
{
    pm_->loadPlugin(name);
    updateList ();
}

void PluginManagerDialog::unload(const QString &name)
{
    pm_->unloadPlugin (name);
    updateList ();
}

void PluginManagerDialog::updateList()
{
    while (ui_->pluginList->rowCount() > 0)
        ui_->pluginList->removeRow(0);
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
}
