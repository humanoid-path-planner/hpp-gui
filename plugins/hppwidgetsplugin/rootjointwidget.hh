#ifndef HPP_GUI_ROOTJOINTWIDGET_HH__
# define HPP_GUI_ROOTJOINTWIDGET_H__

#include <QWidget>
#include <QDoubleSpinBox>

#include <gepetto/viewer/node.h>
#include "hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    class RootJointWidget : public QWidget
    {
      Q_OBJECT
    public:
      RootJointWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);
      void setTransform(hpp::Transform__slice* transform);

    private slots:
      void xChanged(double);
      void yChanged(double);
      void zChanged(double);
      
    private:
      HppWidgetsPlugin* plugin_;
      hpp::Transform__slice* transform_;

      QDoubleSpinBox* xSlider_;
      QDoubleSpinBox* ySlider_;
      QDoubleSpinBox* zSlider_;
    };
  } // namespace gui
} // namespace hpp

#endif /* HPP_GUI_ROOTJOINTWIDGET_HH__ */
