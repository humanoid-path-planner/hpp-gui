//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#ifndef MANIPULATIONNCPICKER_HH
#define MANIPULATIONNCPICKER_HH

#include <hpp/corbaserver/manipulation/graph.hh>

#include "hppwidgetsplugin/numericalconstraintpicker.hh"

namespace hpp {
  namespace gui {
    class HppManipulationWidgetsPlugin;

    class ManipulationNCPicker : public NumericalConstraintPicker
    {
      Q_OBJECT
    private slots:
      virtual void onConfirmClicked();

    public:
      ManipulationNCPicker(HppManipulationWidgetsPlugin* plugin,
                           QWidget* parent = 0);
      virtual ~ManipulationNCPicker();

    protected:
      std::map<std::string, GraphComp> components_;
      QListWidget* listComp_;
    };
  }
}

#endif // MANIPULATIONNCPICKER_HH
