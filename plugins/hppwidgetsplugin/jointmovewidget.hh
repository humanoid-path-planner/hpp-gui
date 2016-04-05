#ifndef HPP_GUI_JOINTMOVEWIDGET_HH__
# define HPP_GUI_JOINTMOVEWIDGET_H__

#include <string>

#include <QDialog>
#include <QDoubleSpinBox>

#include "hppwidgetsplugin.hh"
#include <gepetto/viewer/node.h>

namespace hpp {
  namespace gui {
    class JointMoveWidget : public QDialog
    {
      Q_OBJECT
    public:
      JointMoveWidget(HppWidgetsPlugin* plugin, std::string jointName);

      void setTransform(hpp::Transform__slice* transform);

    private slots:
      void xChanged(double);
      void yChanged(double);
      void zChanged(double);
      
    private:
      HppWidgetsPlugin* plugin_;
      std::string jointName_;
      hpp::Transform__slice* transform_;

      QDoubleSpinBox* xSlider_;
      QDoubleSpinBox* ySlider_;
      QDoubleSpinBox* zSlider_;
    };
  } // namespace gui
} // namespace hpp

#endif /* HPP_GUI_JOINTMOVEWIDGET_HH__ */
