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
    /// Widget that allows to create constraints.
    class ConstraintWidget : public QWidget
    {
        Q_OBJECT

    public:
        ConstraintWidget(HppWidgetsPlugin* plugin, QWidget *parent = 0);
        ~ConstraintWidget();

        /// Add a constraint to the internal vector.
        /// \param constraint constraint to add
        void addConstraint(IConstraint* constraint);

    public slots:
        /// Reload the list of joints.
        void reload();

    private slots:
        /// Create a constraint according to the currently selected.
        void createConstraint();

        /// Set numerical constraints in corbaserver.
        void confirmNumerical();

        /// Apply constraints to the current configuration.
        void applyConstraints();

        /// Reset the numerical constraints.
        void reset();

        /// Fill the second joint list by excluding the first selected.
        /// \param index index of the joint selected
        void firstJointSelect(int index);

        /// Fill the second joint when global frame is selected to be the first joint.
        /// \param action global frame selected or not
        void globalSelected(bool action);

        /// Add the newly created constraint to the list.
        /// \param name name of the constraint
        void createFinished(QString name);

    private:
        HppWidgetsPlugin* plugin_;
        Ui::ConstraintWidget *ui;
        QDockWidget* dock_;
        hpp::Names_t_var joints_;
        int lastInsert_;
        std::vector<IConstraint*> funcs_;

        /// Fill the list of the first joint.
        void fillFirstJoint();
    };
  } // namespace gui
} // namespace hpp


#endif // HPP_GUI_CONSTRAINTWIDGET_HH
