#ifndef HPP_GUI_POSITIONCONSTRAINTWIDGET_HH
#define HPP_GUI_POSITIONCONSTRAINTWIDGET_HH

#include <QDialog>
#include <QDoubleSpinBox>

namespace hpp {
  namespace gui {
    class PositionConstraintWidget : public QDialog
    {
        Q_OBJECT

    public:
        PositionConstraintWidget(QString const& firstJoint, QString const& secondJoint,
                                 QWidget* parent = 0);

    signals:
        void values(QVector<double> vec);

    private:
        QDoubleSpinBox* firstX;
        QDoubleSpinBox* firstY;
        QDoubleSpinBox* firstZ;

        QDoubleSpinBox* secondX;
        QDoubleSpinBox* secondY;
        QDoubleSpinBox* secondZ;

    private slots:
        void confirm();
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_POSITIONCONSTRAINTWIDGET_HH
