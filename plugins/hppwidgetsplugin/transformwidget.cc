#include "hpp/gui/mainwindow.hh"

#include "transformwidget.hh"

namespace hpp {
  namespace gui {
    TransformWidget::TransformWidget(hpp::Transform__slice* transform, std::string const& jointName,
				     QWidget* parent)
      : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint),
	transform_(NULL)
    {
      transform_ = transform;
      rAxis_ = QVector3D(transform_[3], transform_[4], transform_[5]);
      jointName_ = jointName;

      QVBoxLayout* l = new QVBoxLayout;
      this->setLayout(l);

      xSlider_ = new QDoubleSpinBox;
      xSlider_->setMinimum(-DBL_MAX);
      xSlider_->setMaximum(DBL_MAX);
      xSlider_->setSingleStep(0.01);
      xSlider_->setValue(transform_[0]);
      l->addWidget(xSlider_);

      ySlider_ = new QDoubleSpinBox;
      ySlider_->setMinimum(-DBL_MAX);
      ySlider_->setMaximum(DBL_MAX);
      ySlider_->setSingleStep(0.01);
      ySlider_->setValue(transform_[1]);
      l->addWidget(ySlider_);

      zSlider_ = new QDoubleSpinBox;
      zSlider_->setMinimum(-DBL_MAX);
      zSlider_->setMaximum(DBL_MAX);
      zSlider_->setSingleStep(0.01);
      xSlider_->setValue(transform_[2]);
      l->addWidget(zSlider_);

      xQuaternion_ = new QDoubleSpinBox;
      xQuaternion_->setMinimum(-M_PI);
      xQuaternion_->setMaximum(M_PI);
      xQuaternion_->setSingleStep(0.01);
      xQuaternion_->setValue(rAxis_.x());
      l->addWidget(xQuaternion_);

      yQuaternion_ = new QDoubleSpinBox;
      yQuaternion_->setMinimum(-M_PI);
      yQuaternion_->setMaximum(M_PI);
      yQuaternion_->setSingleStep(0.01);
      yQuaternion_->setValue(rAxis_.y());
      l->addWidget(yQuaternion_);

      zQuaternion_ = new QDoubleSpinBox;
      zQuaternion_->setMinimum(-M_PI);
      zQuaternion_->setMaximum(M_PI);
      zQuaternion_->setSingleStep(0.01);
      xQuaternion_->setValue(rAxis_.z());
      l->addWidget(zQuaternion_);

      xSlider_->connect(xSlider_, SIGNAL(valueChanged(double)), this, SLOT(xChanged(double)));
      ySlider_->connect(ySlider_, SIGNAL(valueChanged(double)), this, SLOT(yChanged(double)));
      zSlider_->connect(zSlider_, SIGNAL(valueChanged(double)), this, SLOT(zChanged(double)));

      xQuaternion_->connect(xQuaternion_, SIGNAL(valueChanged(double)),
			    this, SLOT(xRotateChanged(double)));
      yQuaternion_->connect(yQuaternion_, SIGNAL(valueChanged(double)),
			    this, SLOT(yRotateChanged(double)));
      zQuaternion_->connect(zQuaternion_, SIGNAL(valueChanged(double)),
			    this, SLOT(zRotateChanged(double)));

      setWindowTitle(QString::fromStdString("Transform "));
      setAttribute(Qt::WA_DeleteOnClose);
    }

    void TransformWidget::changed(bool axisChanged)
    {
      if (axisChanged) {
	QQuaternion quaternion(rAxis_.length(), rAxis_.normalized());

	transform_[3] = quaternion.x();
	transform_[4] = quaternion.y();
	transform_[5] = quaternion.z();
      }
      emit valueChanged(transform_, jointName_);
    }
    
    void TransformWidget::xChanged(double value)
    {
      transform_[0] = value;
      changed();
    }

    void TransformWidget::yChanged(double value)
    {
      transform_[1] = value;
      changed();
    }

    void TransformWidget::zChanged(double value)
    {
      transform_[2] = value;
      changed();
    }
    
    void TransformWidget::xRotateChanged(double value)
    {
      rAxis_.setX(value);
      changed(true);
    }

    void TransformWidget::yRotateChanged(double value)
    {
      rAxis_.setY(value);
      changed(true);
    }

    void TransformWidget::zRotateChanged(double value)
    {
      rAxis_.setZ(value);
      changed(true);
    }
  }
}
