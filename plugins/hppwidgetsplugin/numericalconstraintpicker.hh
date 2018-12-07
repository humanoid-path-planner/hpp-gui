//
// Copyright (c) CNRS
// Author: Heidy Dallard
//

#ifndef NUMERICALCONSTRAINTPICKER_HH
#define NUMERICALCONSTRAINTPICKER_HH

#include <QWidget>
#include <QListWidgetItem>

#include <omniORB4/CORBA.h>

namespace Ui {
class NumericalConstraintPicker;
}

namespace hpp {
  namespace gui {
    class HppWidgetsPlugin;

    class NumericalConstraintPicker : public QWidget
    {
      Q_OBJECT

    protected slots:
      void onCancelClicked();
      virtual void onConfirmClicked();

    protected:
      void closeEvent(QCloseEvent* event);

    public:
      explicit NumericalConstraintPicker(HppWidgetsPlugin* plugin,
                                         QWidget *parent = 0);
      virtual ~NumericalConstraintPicker();

    protected:
      Ui::NumericalConstraintPicker *ui;
      HppWidgetsPlugin* plugin_;
    };
  } // namespace gui
} // namespace hpp

#endif // NUMERICALCONSTRAINTPICKER_HH
