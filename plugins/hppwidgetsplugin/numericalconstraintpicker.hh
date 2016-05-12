#ifndef NUMERICALCONSTRAINTPICKER_HH
#define NUMERICALCONSTRAINTPICKER_HH

#include <QWidget>
#include <QListWidgetItem>

#include <omniORB4/CORBA.h>
#include "gepetto/gui/meta.hh"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace Ui {
class NumericalConstraintPicker;
}

namespace hpp {
  namespace gui {
    class NumericalConstraintPicker : public QWidget
    {
      Q_OBJECT

    private slots:
      void onCancelClicked();
      void onConfirmClicked();

    public:
      explicit NumericalConstraintPicker(QStringList const& names, HppWidgetsPlugin* plugin,
                                         QWidget *parent = 0);
      ~NumericalConstraintPicker();

    private:
      Ui::NumericalConstraintPicker *ui;
      HppWidgetsPlugin* plugin_;
    };
  } // namespace gui
} // namespace hpp

#endif // NUMERICALCONSTRAINTPICKER_HH
