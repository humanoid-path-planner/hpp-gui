#ifndef LISTJOINTCONSTRAINT_HH
#define LISTJOINTCONSTRAINT_HH

#include <QWidget>

#include "iconstraint.hh"
#include "hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    class ListJointConstraint : public IConstraint
    {
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

    class LockedJointConstraint : public ListJointConstraint
    {
    public:
      LockedJointConstraint(HppWidgetsPlugin* plugin);
      virtual ~LockedJointConstraint();

      virtual QString getName() const;
      virtual void operator()(QString const& name);
    };
  }
}

#endif // LISTJOINTCONSTRAINT_HH
