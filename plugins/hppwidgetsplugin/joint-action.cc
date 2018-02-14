// Copyright (c) 2017, Joseph Mirabel
// Authors: Joseph Mirabel (joseph.mirabel@laas.fr)
//

#include <hppwidgetsplugin/joint-action.hh>

#include <hppwidgetsplugin/jointtreewidget.hh>

namespace hpp {
  namespace gui {
    void JointAction::trigger ()
    {
      if (jointName_.empty()) {
        if (tree_ != NULL)
          emit triggered(tree_->selectedJoint());
      } else
        emit triggered(jointName_);
    }
  } // namespace gui
} // namespace hpp
