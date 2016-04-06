#include "hpp/gui/mainwindow.hh"

#include "jointmovewidget.hh"

namespace hpp {
  namespace gui {
    JointMoveWidget::JointMoveWidget(HppWidgetsPlugin* plugin, std::string jointName)
      : QDialog(NULL),
	transform_(NULL)
    {
      plugin_ = plugin;
      transform_ = plugin->client()->robot()->getJointPositionInParentFrame(jointName.c_str());
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

      xSlider_->connect(xSlider_, SIGNAL(valueChanged(double)), this, SLOT(xChanged(double)));
      ySlider_->connect(ySlider_, SIGNAL(valueChanged(double)), this, SLOT(yChanged(double)));
      ySlider_->connect(zSlider_, SIGNAL(valueChanged(double)), this, SLOT(zChanged(double)));

      setWindowTitle(QString::fromStdString("Move " + jointName_));
      setAttribute(Qt::WA_DeleteOnClose);
    }

    void JointMoveWidget::changed()
    {
      plugin_->client()->robot()->setJointPositionInParentFrame(jointName_.c_str(), transform_);
      MainWindow::instance()->requestApplyCurrentConfiguration();
    }
    
    void JointMoveWidget::xChanged(double value)
    {
      transform_[0] = value;
      changed();
    }

    void JointMoveWidget::yChanged(double value)
    {
      transform_[1] = value;
      changed();
    }

    void JointMoveWidget::zChanged(double value)
    {
      transform_[2] = value;
      changed();
    }
  }
}
