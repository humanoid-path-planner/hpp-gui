#ifndef HPP_GUI_MANIPULATIONLOCKEDJOINT_HH
#define HPP_GUI_MANIPULATIONLOCKEDJOINT_HH

#include "hppwidgetsplugin/listjointconstraint.hh"

namespace hpp {
  namespace gui {
    class HppManipulationWidgetsPlugin;

    class ManipulationLockedJoint : public ListJointConstraint
    {
      Q_OBJECT
    public:
      ManipulationLockedJoint(HppManipulationWidgetsPlugin* plugin);
      virtual ~ManipulationLockedJoint();

      virtual QString getName() const;
      virtual void operator()(QString const& name);
    };
  }
}

#endif // HPP_GUI_MANIPULATIONLOCKEDJOINT_HH
