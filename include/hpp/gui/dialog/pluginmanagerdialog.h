#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H

#include <QDialog>
#include <QMap>
#include <QPluginLoader>
#include <QTableWidgetItem>
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

  template <typename Interface> Interface* getFirstOf ();

  template <typename Interface> QList <Interface*> get ();

  static QIcon icon (const QPluginLoader* pl);

  static QString status (const QPluginLoader* pl);

private:
  bool loadPlugin (const QString& name);

  template <typename Interface>
  static const Interface* const_instance_cast (const QPluginLoader* pl);

  QMap <QString, QPluginLoader*> plugins_;
};

class PluginManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PluginManagerDialog(PluginManager* pm, QWidget *parent = 0);
  ~PluginManagerDialog();

public slots:
  void onItemChanged (QTableWidgetItem* current, QTableWidgetItem* previous);

private:
  Ui::PluginManagerDialog *ui_;

  PluginManager* pm_;
};

template <typename Interface>
Interface* PluginManager::getFirstOf ()
{
  foreach (QPluginLoader* p, plugins_) {
      Interface* pi = qobject_cast <Interface*> (p->instance());
      if (pi) return pi;
    }
  return NULL;
}

template <typename Interface>
QList <Interface*> PluginManager::get ()
{
  QList <Interface*> list;
  foreach (QPluginLoader* p, plugins_) {
      Interface* pi = qobject_cast <Interface*> (p->instance());
      if (pi) list.append(pi);
    }
  return list;
}

template <typename Interface>
const Interface* PluginManager::const_instance_cast (const QPluginLoader* pl)
{
    return (const Interface*) qobject_cast <Interface*> (const_cast <QPluginLoader*>(pl)->instance());
}

#endif // PLUGINMANAGERDIALOG_H
