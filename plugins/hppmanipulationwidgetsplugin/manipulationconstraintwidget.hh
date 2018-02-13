//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

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
					     
    public slots:
      virtual void refresh();

    private slots:
      virtual void applyConstraints();
      virtual void reset();
      virtual void confirmNumerical();

    protected:
    };
  }
}

#endif // HPP_GUI_MANIPULATIONCONSTRAINTWIDGET_HH
