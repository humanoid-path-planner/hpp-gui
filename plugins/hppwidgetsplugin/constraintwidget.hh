//
// Copyright (c) CNRS
// Author: Heidy Dallard
//

#ifndef HPP_GUI_CONSTRAINTWIDGET_HH
#define HPP_GUI_CONSTRAINTWIDGET_HH

#include <QWidget>

#include "hppwidgetsplugin/iconstraint.hh"

class QDockWidget;

namespace Ui {
  class ConstraintWidget;
}

namespace hpp {
  namespace gui {
    class HppWidgetsPlugin;

    /// Widget that allows to create constraints.
    class ConstraintWidget : public QWidget
    {
        Q_OBJECT

    public:
        ConstraintWidget(HppWidgetsPlugin* plugin, QWidget *parent = 0);
        virtual ~ConstraintWidget();

    public slots:
        /// Reload the list of joints.
        void reload();

        /// Add a constraint to the internal vector.
        /// \param constraint constraint to add
        void addConstraint(IConstraint* constraint);

      virtual void refresh();

    private slots:
        /// Create a constraint according to the currently selected.
        void createConstraint();

        /// Set numerical constraints in corbaserver.
        virtual void confirmNumerical();

        /// Apply constraints to the current configuration.
        virtual void applyConstraints();

        /// Reset the numerical constraints.
        virtual void reset();

        /// Add the newly created constraint to the list.
        /// \param name name of the constraint
        void onConstraintCreated(QString name);

        void onFinished();

        void typeChanged(int index);

    protected:
        HppWidgetsPlugin* plugin_;
        Ui::ConstraintWidget *ui;
        QDockWidget* dock_;
        int lastInsert_;
        std::vector<IConstraint*> funcs_;
        bool haveWidget;
    };
  } // namespace gui
} // namespace hpp


#endif // HPP_GUI_CONSTRAINTWIDGET_HH
