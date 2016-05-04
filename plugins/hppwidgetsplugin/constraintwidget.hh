#ifndef HPP_GUI_CONSTRAINTWIDGET_HH
#define HPP_GUI_CONSTRAINTWIDGET_HH

#include <QWidget>
#include <QDockWidget>

#include "hppwidgetsplugin/hppwidgetsplugin.hh"
#include "hppwidgetsplugin/iconstraint.hh"

namespace Ui {
  class ConstraintWidget;
}

namespace hpp {
  namespace gui {

    class ConstraintWidget : public QWidget
    {
        Q_OBJECT

    public:
        ConstraintWidget(HppWidgetsPlugin* plugin, QWidget *parent = 0);
        ~ConstraintWidget();

        void addConstraint(IConstraint* constraint);

    public slots:
        void reload();

    private slots:
        void createConstraint();
        void confirmNumerical();
        void applyConstraints();
        void reset();
        void firstJointSelect(int index);
        void globalSelected(bool action);
        void createFinished(QString);

    private:
        HppWidgetsPlugin* plugin_;
        Ui::ConstraintWidget *ui;
        QDockWidget* dock_;
        hpp::Names_t_var joints_;
        int lastInsert_;
        std::vector<IConstraint*> funcs_;

        void fillFirstJoint();
    };
  } // namespace gui
} // namespace hpp


#endif // HPP_GUI_CONSTRAINTWIDGET_HH
