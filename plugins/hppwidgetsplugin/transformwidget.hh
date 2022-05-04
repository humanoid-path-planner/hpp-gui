//
// Copyright (c) CNRS
// Author: Heidy Dallard
//

#ifndef HPP_GUI_TRANSFORMWIDGET_HH__
#define HPP_GUI_TRANSFORMWIDGET_H__

#include <gepetto/viewer/node.h>

#include <QDialog>
#include <QDoubleSpinBox>
#include <QVector3D>
#include <string>

#include "gepetto/gui/mainwindow.hh"
#include "hppwidgetsplugin.hh"

namespace hpp {
namespace gui {
class TransformWidget : public QDialog {
  Q_OBJECT
 public:
  TransformWidget(hpp::Transform__slice* transform,
                  std::string const& jointName, QWidget* parent,
                  bool doPosition = true, bool doQuaternion = true);

 signals:
  void valueChanged(hpp::Transform__slice* transform,
                    std::string const& jointName);

 private slots:
  void xChanged(double);
  void yChanged(double);
  void zChanged(double);
  void xRotateChanged(double);
  void yRotateChanged(double);
  void zRotateChanged(double);

 private:
  void changed(bool axisChanged = false);

  QVector3D rAxis_;
  hpp::Transform__slice* transform_;
  std::string jointName_;

  QDoubleSpinBox* xSlider_;
  QDoubleSpinBox* ySlider_;
  QDoubleSpinBox* zSlider_;

  QDoubleSpinBox* xQuaternion_;
  QDoubleSpinBox* yQuaternion_;
  QDoubleSpinBox* zQuaternion_;
};
}  // namespace gui
}  // namespace hpp

#endif /* HPP_GUI_TRANSFORMWIDGET_HH__ */
