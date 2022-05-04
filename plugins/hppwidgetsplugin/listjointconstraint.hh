//
// Copyright (c) CNRS
// Author: Heidy Dallard
//

#ifndef LISTJOINTCONSTRAINT_HH
#define LISTJOINTCONSTRAINT_HH

#include "hppwidgetsplugin.hh"
#include "iconstraint.hh"

class QListWidget;
class QWidget;

namespace hpp {
namespace gui {
class ListJointConstraint : public IConstraint {
 public:
  ListJointConstraint(HppWidgetsPlugin* plugin);
  virtual ~ListJointConstraint();

  virtual QWidget* getWidget() const;
  virtual void reload();

 protected:
  HppWidgetsPlugin* plugin_;
  QWidget* widget_;
  QListWidget* jointList_;
};

class LockedJointConstraint : public ListJointConstraint {
 public:
  LockedJointConstraint(HppWidgetsPlugin* plugin);
  virtual ~LockedJointConstraint();

  virtual QString getName() const;
  virtual void operator()(QString const& name);
};
}  // namespace gui
}  // namespace hpp

#endif  // LISTJOINTCONSTRAINT_HH
