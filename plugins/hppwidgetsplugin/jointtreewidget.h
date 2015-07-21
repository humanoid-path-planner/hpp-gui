#ifndef JOINTTREEWIDGET_H
#define JOINTTREEWIDGET_H

#include <QWidget>

#include <hppwidgetsplugin.hh>

namespace Ui {
  class JointTreeWidget;
}

class JointTreeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit JointTreeWidget(HppWidgetsPlugin *plugin, QWidget *parent = 0);

  virtual ~JointTreeWidget ();

  const QPushButton* refreshButton () const;

  void dockWidget (QDockWidget* dock);

signals:

public slots:
  void customContextMenu (const QPoint& pos);
  void addJointToTree (const std::string name, JointTreeItem *parent);
  void selectJoint (const std::string& jointName);

  void reload ();

private:
  void reset ();

  HppWidgetsPlugin* plugin_;
  Ui::JointTreeWidget* ui_;

  QStandardItemModel* model_;
  QDockWidget* dock_;
};

#endif // JOINTTREEWIDGET_H
