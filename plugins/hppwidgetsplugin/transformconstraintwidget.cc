//
// Copyright (c) CNRS
// Authors: Heidy Dallard
//

#include <limits>

#include "hppwidgetsplugin/transformconstraintwidget.hh"
#include "hppwidgetsplugin/ui_transformconstraintwidget.h"

namespace hpp {
  namespace gui {
    TransformConstraintWidget::TransformConstraintWidget(QString const& firstJoint,
                                                         QString const& secondJoint,
                                                         bool doPosition,
                                                         bool doOrientation,
                                                         bool isPositionConstraint,
                                                         QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TransformConstraintWidget)
    {
      ui->setupUi(this);
      length_ = 0;

      if (!doPosition) {
        ui->positionGroup->setVisible(false);
      }
      else {
        ui->xPositionWidget->setVisible(false);
        ui->yPositionWidget->setVisible(false);
        ui->zPositionWidget->setVisible(false);
        length_ += 3;
      }
      if (!doOrientation) {
        ui->orientationGroup->setVisible(false);
      }
      else {
        ui->xOrientationValue->setVisible(false);
        ui->yOrientationValue->setVisible(false);
        ui->zOrientationValue->setVisible(false);
        length_ += 3;
      }

      if (!isPositionConstraint || (isPositionConstraint && doOrientation)) {
        ui->secondXPosition->setVisible(false);
        ui->secondYPosition->setVisible(false);
        ui->secondZPosition->setVisible(false);
        isPositionConstraint_ = false;
      }
      else {
        length_ += 3;
        isPositionConstraint_ = true;
      }

      ui->secondJointLabel->setText((secondJoint == "") ? "Global frame" : secondJoint);
      ui->firstJointLabel->setText((firstJoint == "") ? "Global frame" : firstJoint);

      positionEnabled_ = doPosition;
      orientationEnabled_ = doOrientation;

      this->layout()->setSizeConstraint(QLayout::SetFixedSize);
      connect(ui->confirmButton, SIGNAL(clicked()), SLOT(onClick()));
    }

    TransformConstraintWidget::~TransformConstraintWidget()
    {
      delete ui;
    }

    void TransformConstraintWidget::onClick()
    {
      QVector<double> vecDouble(length_, 0);
      QVector<bool> vecBool((positionEnabled_ && orientationEnabled_) ? 6 : 3, 0);
      int i = 0;

      if (positionEnabled_) {
        vecDouble[i] = ui->firstXPosition->value();
        vecDouble[i + 1] = ui->firstYPosition->value();
        vecDouble[i + 2] = ui->firstZPosition->value();
        vecBool[i] = ui->xPositionCheck->isChecked();
        vecBool[i + 1] = ui->yPositionCheck->isChecked();
        vecBool[i + 2] = ui->zPositionCheck->isChecked();
        i += 3;
      }
      if (orientationEnabled_) {
        vecDouble[i] = ui->xOrientationValue->value();
        vecDouble[i + 1] = ui->yOrientationValue->value();
        vecDouble[i + 2] = ui->zOrientationValue->value();
        vecBool[i] = ui->xOrientationCheck->isChecked();
        vecBool[i + 1] = ui->yOrientationCheck->isChecked();
        vecBool[i + 2] = ui->zOrientationCheck->isChecked();
        i += 3;
      }
      if (isPositionConstraint_) {
        vecDouble[i] = ui->secondXPosition->value();
        vecDouble[i + 1] = ui->secondYPosition->value();
        vecDouble[i + 2] = ui->secondZPosition->value();
      }
      emit finished(std::make_pair<QVector<double>, QVector<bool> >(vecDouble, vecBool));
      this->deleteLater();
      this->close();
    }
  }
}
