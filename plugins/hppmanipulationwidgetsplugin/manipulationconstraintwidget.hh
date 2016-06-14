#ifndef HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH
#define HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH

#include "hpp/corbaserver/manipulation/graph.hh"

#include "hppwidgetsplugin/constraintwidget.hh"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    class ManipulationConstraintWidget : public ConstraintWidget
    {
      Q_OBJECT
    public:
      ManipulationConstraintWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);
      virtual ~ManipulationConstraintWidget();

      virtual void applyConstraints();
      virtual void reset();
      virtual void confirmNumerical();

    protected:
    };
  }
}
class ManipulationConstraintWidget
{
public:
    ManipulationConstraintWidget();
};

#endif // HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH
