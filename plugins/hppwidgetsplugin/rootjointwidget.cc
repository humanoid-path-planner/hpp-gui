#include "rootjointwidget.hh"

namespace hpp {
  namespace gui {
    RootJointWidget::RootJointWidget(HppWidgetsPlugin* plugin, QWidget* parent)
      : QWidget(parent),
	plugin_(NULL),
	transform_(NULL)
    {
      QVBoxLayout* l = new QVBoxLayout;
      this->setLayout(l);

      xSlider_ = new QDoubleSpinBox;
      xSlider_->setMinimum(-DBL_MAX);
      xSlider_->setMaximum(DBL_MAX);
      xSlider_->setSingleStep(0.01);
      l->addWidget(xSlider_);

      ySlider_ = new QDoubleSpinBox;
      ySlider_->setMinimum(-DBL_MAX);
      ySlider_->setMaximum(DBL_MAX);
      ySlider_->setSingleStep(0.01);
      l->addWidget(ySlider_);

      zSlider_ = new QDoubleSpinBox;
      zSlider_->setMinimum(-DBL_MAX);
      zSlider_->setMaximum(DBL_MAX);
      zSlider_->setSingleStep(0.01);
      l->addWidget(zSlider_);

      xSlider_->connect(xSlider_, SIGNAL(valueChanged(double)), this, SLOT(xChanged(double)));
      ySlider_->connect(ySlider_, SIGNAL(valueChanged(double)), this, SLOT(yChanged(double)));
      ySlider_->connect(zSlider_, SIGNAL(valueChanged(double)), this, SLOT(zChanged(double)));

      plugin_ = plugin;
    }

    void RootJointWidget::setTransform(hpp::Transform__slice* transform)
    {
      if (transform) {
	transform_ = transform;
	xSlider_->setValue(transform_[0]);
	ySlider_->setValue(transform_[1]);
	zSlider_->setValue(transform_[2]);
      }
    }

    void RootJointWidget::xChanged(double value)
    {
      if (transform_) {
	transform_[0] = value;
	plugin_->client()->robot()->setRootJointPosition(transform_);
      }
    }

    void RootJointWidget::yChanged(double value)
    {
      if (transform_) {
	transform_[1] = value;
	plugin_->client()->robot()->setRootJointPosition(transform_);
      }
    }

    void RootJointWidget::zChanged(double value)
    {
      if (transform_) {
	transform_[2] = value;
	plugin_->client()->robot()->setRootJointPosition(transform_);
      }
    }
  }
}
