#ifndef HPP_GUI_TRANSFORMCONSTRAINTWIDGET_HH
#define HPP_GUI_TRANSFORMCONSTRAINTWIDGET_HH

#include <QWidget>

#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace Ui {
class TransformConstraintWidget;
}

namespace hpp {
  namespace gui {
    class TransformConstraintWidget : public QWidget
    {
      Q_OBJECT

    public:
      explicit TransformConstraintWidget(QString const& firstJoint, QString const& secondJoint,
                                         bool doPosition = true, bool doOrientation = true,
                                         bool isPositionConstraint = false,
                                         QWidget *parent = 0);
      ~TransformConstraintWidget();

    signals:
      void finished(std::pair<QVector<double>, QVector<bool> > result);

    private slots:
      void onClick();

    private:
      Ui::TransformConstraintWidget *ui;
      bool positionEnabled_;
      bool orientationEnabled_;
      bool isPositionConstraint_;
      int length_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_TRANSFORMCONSTRAINTWIDGET_HH
