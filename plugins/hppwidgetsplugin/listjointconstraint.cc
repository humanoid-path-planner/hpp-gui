//
// Copyright (c) CNRS
// Authors: Heidy Dallard
//

#include "listjointconstraint.hh"

#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QListWidget>

namespace hpp {
  namespace gui {
    ListJointConstraint::ListJointConstraint(HppWidgetsPlugin *plugin)
    {
      plugin_ = plugin;
      widget_ = new QWidget;
      QVBoxLayout* layout = new QVBoxLayout(widget_);
      jointList_ = new QListWidget(widget_);

      layout->addWidget(jointList_);
      jointList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    ListJointConstraint::~ListJointConstraint()
    {
      delete widget_;
    }

    QWidget* ListJointConstraint::getWidget() const
    {
      return widget_;
    }

    void ListJointConstraint::reload()
    {
      hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames();

      jointList_->clear();
      for (unsigned i = 0; i < joints->length(); i++) {
        jointList_->addItem(joints[i].in());
      }
    }

    LockedJointConstraint::LockedJointConstraint(HppWidgetsPlugin *plugin)
        : ListJointConstraint(plugin)
    {
      widget_->layout()->addWidget(new QLabel("The joints will be locked in their current position.", widget_));
    }

    LockedJointConstraint::~LockedJointConstraint()
    {
    }

    QString LockedJointConstraint::getName() const
    {
      return "Lock joints";
    }

    void LockedJointConstraint::operator ()(QString const&)
    {
      QList<QListWidgetItem *> selected = jointList_->selectedItems();

      foreach(QListWidgetItem* item, selected) {
          std::string jointName = item->text().toStdString();
          std::string lockName = "lock_" + jointName;
          hpp::floatSeq_var config = plugin_->client()->robot()->getJointConfig(jointName.c_str());
          plugin_->client()->problem()->createLockedJoint(lockName.c_str(),
                                                          jointName.c_str(),
                                                          config.in());
          emit constraintCreated(lockName.c_str());
      }
    }
  }
}
