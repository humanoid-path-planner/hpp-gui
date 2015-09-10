#ifndef JOINTTREEWIDGET_H
#define JOINTTREEWIDGET_H

#include <QWidget>

#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace Ui {
  class JointTreeWidget;
}

class JointTreeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit JointTreeWidget(HppWidgetsPlugin *plugin, QWidget *parent = 0);

  virtual ~JointTreeWidget ();

  void dockWidget (QDockWidget* dock);

  std::string selectedJoint ();

signals:

public slots:
  void customContextMenu (const QPoint& pos);
  void addJointToTree (const std::string name, JointTreeItem *parent);
  void selectJoint (const std::string& jointName);
  void openJointBoundDialog (const std::string jointName);

  void reload ();

private slots:
  void resize (const QModelIndex index);

private:
  void reset ();

  HppWidgetsPlugin* plugin_;
  Ui::JointTreeWidget* ui_;

  QStandardItemModel* model_;
  QDockWidget* dock_;
};

#endif // JOINTTREEWIDGET_H
