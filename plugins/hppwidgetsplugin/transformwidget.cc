#include "gepetto/gui/mainwindow.hh"

#include "transformwidget.hh"

namespace hpp {
  namespace gui {
    TransformWidget::TransformWidget(hpp::Transform__slice* transform, std::string const& jointName,
                                     QWidget* parent, bool doPosition, bool doQuaternion)
      : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint),
	transform_(NULL)
    {
      transform_ = transform;
      QQuaternion q (transform_[3], transform_[4], transform_[5], transform_[6]);
      q.normalize();
      rAxis_ = q.vector().normalized() * 2 * std::atan2(q.vector().length(), q.scalar());
      jointName_ = jointName;
      QVBoxLayout* l = new QVBoxLayout;
      this->setLayout(l);

      if (doPosition) {
        l->addWidget(new QLabel("Position", this));
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

        xSlider_->connect(xSlider_, SIGNAL(valueChanged(double)), this, SLOT(xChanged(double)));
        ySlider_->connect(ySlider_, SIGNAL(valueChanged(double)), this, SLOT(yChanged(double)));
        zSlider_->connect(zSlider_, SIGNAL(valueChanged(double)), this, SLOT(zChanged(double)));
      }

      if (doQuaternion) {
        l->addWidget(new QLabel("Quaternion", this));
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

	xQuaternion_->connect(xQuaternion_, SIGNAL(valueChanged(double)),
			      this, SLOT(xRotateChanged(double)));
	yQuaternion_->connect(yQuaternion_, SIGNAL(valueChanged(double)),
			      this, SLOT(yRotateChanged(double)));
	zQuaternion_->connect(zQuaternion_, SIGNAL(valueChanged(double)),
			      this, SLOT(zRotateChanged(double)));
      }

      setWindowTitle(QString::fromStdString("Transform "));
      setAttribute(Qt::WA_DeleteOnClose);
    }

    void TransformWidget::changed(bool axisChanged)
    {
      if (axisChanged) {
        QQuaternion quaternion;
        if (!rAxis_.isNull()) {
          quaternion = QQuaternion::fromAxisAndAngle(rAxis_.normalized(), rAxis_.length());
          const double theta = rAxis_.length();
          quaternion = QQuaternion(std::cos(theta/2), std::sin(theta/2) * rAxis_ / theta);
        }

        qDebug()<< rAxis_;
        qDebug()<< quaternion;
        transform_[3] = quaternion.scalar();
        transform_[4] = quaternion.x();
        transform_[5] = quaternion.y();
        transform_[6] = quaternion.z();
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
