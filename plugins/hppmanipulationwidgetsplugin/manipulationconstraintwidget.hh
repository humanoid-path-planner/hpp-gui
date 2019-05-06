//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH
#define HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH

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
					     
    private slots:
      virtual void applyConstraints();
      virtual void reset();
    protected:
    };
  }
}

#endif // HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH
