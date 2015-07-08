#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H

#include <QDialog>
#include <QMap>
#include <QPluginLoader>
#include <QListWidgetItem>
#include <QDebug>

namespace Ui {
  class PluginManagerDialog;
}

class PluginManager {
public:
  typedef QPair <QString, QPluginLoader*> Pair;
  typedef QMap <QString, QPluginLoader*> Map;

  ~PluginManager () {
    qDeleteAll (plugins_);
  }

  const QMap <QString, QPluginLoader*>& plugins () const {
    return plugins_;
  }

  bool add (const QString& name, QWidget* parent = NULL, bool load = false);

  template <typename Interface> Interface* get ();

private:
  bool loadPlugin (const QString& name);

  QMap <QString, QPluginLoader*> plugins_;
};

class PluginManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PluginManagerDialog(PluginManager* pm, QWidget *parent = 0);
  ~PluginManagerDialog();

public slots:
  void onItemChanged (QListWidgetItem* current, QListWidgetItem* previous);

private:
  Ui::PluginManagerDialog *ui_;

  PluginManager* pm_;
};

template <typename Interface>
Interface* PluginManager::get ()
{
  foreach (QPluginLoader* p, plugins_) {
      Interface* pi = qobject_cast <Interface*> (p->instance());
      if (pi) return pi;
    }
  return NULL;
}

#endif // PLUGINMANAGERDIALOG_H
