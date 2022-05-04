//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include "transformwidget.hh"

#include <QVBoxLayout>

#include "gepetto/gui/mainwindow.hh"

namespace hpp {
namespace gui {
namespace {
QDoubleSpinBox* makeSpinBox(double val, TransformWidget* recv, const char* slot,
                            double bound) {
  QDoubleSpinBox* sb = new QDoubleSpinBox;
  sb->setDecimals(10 /*DBL_MAX_10_EXP + DBL_DIG*/);
  sb->setMinimum(-bound);
  sb->setMaximum(bound);
  sb->setSingleStep(0.01);
  sb->setValue(val);
  recv->connect(sb, SIGNAL(valueChanged(double)), slot);
  return sb;
}
}  // namespace

TransformWidget::TransformWidget(hpp::Transform__slice* transform,
                                 std::string const& jointName, QWidget* parent,
                                 bool doPosition, bool doQuaternion)
    : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint), transform_(NULL) {
  transform_ = transform;
  QQuaternion q((float)transform_[6], (float)transform_[3],
                (float)transform_[4], (float)transform_[5]);
  q.normalize();
  qDebug() << q;
  rAxis_ =
      q.vector().normalized() * 2 * std::atan2(q.vector().length(), q.scalar());
  jointName_ = jointName;
  QVBoxLayout* l = new QVBoxLayout;
  this->setLayout(l);

  if (doPosition) {
    l->addWidget(new QLabel("Position", this));
    xSlider_ =
        makeSpinBox(transform_[0], this, SLOT(xChanged(double)), DBL_MAX);
    l->addWidget(xSlider_);

    ySlider_ =
        makeSpinBox(transform_[1], this, SLOT(yChanged(double)), DBL_MAX);
    l->addWidget(ySlider_);

    zSlider_ =
        makeSpinBox(transform_[2], this, SLOT(zChanged(double)), DBL_MAX);
    l->addWidget(zSlider_);
  }

  if (doQuaternion) {
    l->addWidget(new QLabel("Quaternion", this));
    xQuaternion_ =
        makeSpinBox(rAxis_.x(), this, SLOT(xRotateChanged(double)), M_PI);
    l->addWidget(xQuaternion_);

    yQuaternion_ =
        makeSpinBox(rAxis_.y(), this, SLOT(yRotateChanged(double)), M_PI);
    l->addWidget(yQuaternion_);

    zQuaternion_ =
        makeSpinBox(rAxis_.z(), this, SLOT(zRotateChanged(double)), M_PI);
    l->addWidget(zQuaternion_);
  }

  setWindowTitle(QString::fromStdString("Transform "));
  setAttribute(Qt::WA_DeleteOnClose);
}

void TransformWidget::changed(bool axisChanged) {
  if (axisChanged) {
    QQuaternion quaternion;
    if (!rAxis_.isNull()) {
      const float theta = rAxis_.length();
      quaternion = QQuaternion(std::cos(theta / 2),
                               std::sin(theta / 2) * rAxis_ / theta);
    }

    transform_[6] = quaternion.scalar();
    transform_[3] = quaternion.x();
    transform_[4] = quaternion.y();
    transform_[5] = quaternion.z();
  }
  emit valueChanged(transform_, jointName_);
}

void TransformWidget::xChanged(double value) {
  transform_[0] = value;
  changed();
}

void TransformWidget::yChanged(double value) {
  transform_[1] = value;
  changed();
}

void TransformWidget::zChanged(double value) {
  transform_[2] = value;
  changed();
}

void TransformWidget::xRotateChanged(double value) {
  rAxis_.setX((float)value);
  changed(true);
}

void TransformWidget::yRotateChanged(double value) {
  rAxis_.setY((float)value);
  changed(true);
}

void TransformWidget::zRotateChanged(double value) {
  rAxis_.setZ((float)value);
  changed(true);
}
}  // namespace gui
}  // namespace hpp
