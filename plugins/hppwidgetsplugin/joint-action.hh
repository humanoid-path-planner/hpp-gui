//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#ifndef HPP_GUI_JOINT_ACTION_HH
#define HPP_GUI_JOINT_ACTION_HH

#include <QAction>

namespace hpp {
namespace gui {
class JointTreeWidget;

class JointAction : public QAction {
  Q_OBJECT

 public:
  JointAction(const QString& actionName, const std::string& jointName,
              QObject* parent)
      : QAction(actionName, parent), jointName_(jointName), tree_(NULL) {
    connect(this, SIGNAL(triggered(bool)), SLOT(trigger()));
  }

  JointAction(const QString& actionName, JointTreeWidget* tree, QObject* parent)
      : QAction(actionName, parent), tree_(tree) {
    connect(this, SIGNAL(triggered(bool)), SLOT(trigger()));
  }

 signals:
  void triggered(const std::string& jointName);

 private slots:
  void trigger();

 private:
  const std::string jointName_;
  JointTreeWidget* tree_;
};
}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_JOINT_ACTION_HH
