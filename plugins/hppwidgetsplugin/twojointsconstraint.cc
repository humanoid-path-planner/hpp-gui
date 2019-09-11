//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#include "twojointsconstraint.hh"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QString>
#include <QWidget>

#include <omniORB4/CORBA.h>

#include <gepetto/viewer/node.h>
#include <gepetto/gui/mainwindow.hh>

#include "hppwidgetsplugin/transformconstraintwidget.hh"

namespace hpp {
  namespace gui {
    ATwoJointConstraint::ATwoJointConstraint(HppWidgetsPlugin* plugin)
    {
        plugin_ = plugin;
        widget_ = new QWidget;
        QFormLayout* layout = new QFormLayout(widget_);
        firstJoint_ = new QComboBox(widget_);
        secondJoint_ = new QComboBox(widget_);
        globalFirst_ = new QCheckBox(widget_);
        globalSecond_ = new QCheckBox(widget_);

        layout->addRow(new QLabel("First Joint"), firstJoint_);
        layout->addRow(NULL, globalFirst_);
        layout->addRow(new QLabel("Second Joint"), secondJoint_);
        layout->addRow(NULL, globalSecond_);

        connect(firstJoint_, SIGNAL(currentIndexChanged(int)), SLOT(firstJointSelect(int)));
        connect(globalFirst_, SIGNAL(toggled(bool)), SLOT(globalSelected(bool)));
        connect(globalFirst_, SIGNAL(toggled(bool)), firstJoint_, SLOT(setDisabled(bool)));
        connect(globalSecond_, SIGNAL(toggled(bool)), secondJoint_, SLOT(setDisabled(bool)));
    }

    ATwoJointConstraint::~ATwoJointConstraint()
    {
        delete widget_;
    }

    void ATwoJointConstraint::firstJointSelect(int index)
    {
        secondJoint_->clear();
        for (CORBA::ULong i = 0; i < joints_->length(); i++) {
          if (int(i) != index) secondJoint_->addItem(joints_[i].in());
        }
    }

    void ATwoJointConstraint::globalSelected(bool action)
    {
        if (action) {
          firstJointSelect(-1);
        }
        else {
          firstJointSelect(firstJoint_->currentIndex());
        }
    }

    void ATwoJointConstraint::reload()
    {
      joints_ = plugin_->client()->robot()->getAllJointNames();
      firstJoint_->clear();
      for (unsigned i = 0; i < joints_->length(); ++i) {
        firstJoint_->addItem(joints_[i].in());
      }
    }

    QWidget* ATwoJointConstraint::getWidget() const
    {
      return widget_;
    }

    PositionConstraint::PositionConstraint(HppWidgetsPlugin *plugin)
        : ATwoJointConstraint(plugin)
    {
        plugin_ = plugin;
    }

    PositionConstraint::~PositionConstraint()
    {
    }

    QString PositionConstraint::getName() const
    {
      return "Position";
    }

    void PositionConstraint::operator ()(QString const& name)
    {
      QString firstJoint = (firstJoint_->isEnabled()) ? firstJoint_->currentText() : "";
      QString secondJoint = (secondJoint_->isEnabled()) ? secondJoint_->currentText() : "";

      if (firstJoint == secondJoint) {
        QMessageBox::information(NULL, "hpp-gui", "You have to select two different joints");
        return ;
      }
      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     true, false, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
              SLOT(getPositionConstraint(std::pair<QVector<double>,QVector<bool> >)));
      tcw->show();
      name_ = name;
      firstJointName_ = firstJoint;
      secondJointName_ = secondJoint;
    }

    void PositionConstraint::getPositionConstraint(std::pair<QVector<double>, QVector<bool> > result)
    {
        hpp::floatSeq_var first = new hpp::floatSeq;
        hpp::floatSeq_var second = new hpp::floatSeq;
        hpp::boolSeq_var boolSeq = new hpp::boolSeq;

        first->length(3);
        second->length(3);
        boolSeq->length(3);
        first[0] = result.first[0];
        first[1] = result.first[1];
        first[2] = result.first[2];
        second[0] = result.first[3];
        second[1] = result.first[4];
        second[2] = result.first[5];
        boolSeq[0] = result.second[0];
        boolSeq[1] = result.second[1];
        boolSeq[2] = result.second[2];

        plugin_->client()->problem()->
              createPositionConstraint(to_corba(name_),
                                       to_corba(firstJointName_),
                                       to_corba(secondJointName_),
                                       first.in(), second.in(), boolSeq.in());
        emit constraintCreated(name_);
        emit finished();
    }

    OrientationConstraint::OrientationConstraint(HppWidgetsPlugin *plugin)
        : ATwoJointConstraint(plugin)
    {
      plugin_ = plugin;
    }

    OrientationConstraint::~OrientationConstraint()
    {
    }

    QString OrientationConstraint::getName() const
    {
      return "Orientation";
    }

    void OrientationConstraint::getOrientationConstraint(std::pair<QVector<double>, QVector<bool> > result)
    {
      QVector3D vec3d((float)result.first[0], (float)result.first[1], (float)result.first[2]);
      QQuaternion qtQuat;
      hpp::Quaternion__var quat = new hpp::Quaternion_;
      hpp::boolSeq_var boolSeq = new hpp::boolSeq;

      if (!vec3d.isNull()) {
          qtQuat = QQuaternion::fromAxisAndAngle(vec3d.normalized(), vec3d.length());
          const float theta = (float) vec3d.length();
          qtQuat = QQuaternion(std::cos(theta/2), std::sin(theta/2) * vec3d / theta);
      }

      boolSeq->length(3);
      quat.inout()[0] = qtQuat.scalar();
      quat.inout()[1] = qtQuat.x();
      quat.inout()[2] = qtQuat.y();
      quat.inout()[3] = qtQuat.z();
      boolSeq[0] = result.second[0];
      boolSeq[1] = result.second[1];
      boolSeq[2] = result.second[2];

      plugin_->client()->problem()->
            createOrientationConstraint(to_corba(name_),
                                        to_corba(firstJoint_->currentText()),
                                        to_corba(secondJoint_->currentText()),
                                        quat.in(), boolSeq.in());
      emit constraintCreated(name_);
      emit finished();
    }

    void OrientationConstraint::operator ()(QString const& name)
    {
      QString firstJoint = (firstJoint_->isEnabled()) ? firstJoint_->currentText() : "";
      QString secondJoint = (secondJoint_->isEnabled()) ? secondJoint_->currentText() : "";

      if (firstJoint == secondJoint) {
        QMessageBox::information(NULL, "hpp-gui", "You have to select two different joints");
        return ;
      }

      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     false, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
                   SLOT(getOrientationConstraint(std::pair<QVector<double>,QVector<bool> >)));
      tcw->show();
      name_ = name;
      firstJointName_ = firstJoint;
      secondJointName_ = secondJoint;
    }

    TransformConstraint::TransformConstraint(HppWidgetsPlugin *plugin)
        : ATwoJointConstraint(plugin)
    {
      plugin_ = plugin;
    }

    TransformConstraint::~TransformConstraint()
    {
    }

    QString TransformConstraint::getName() const
    {
      return "Transformation";
    }

    void TransformConstraint::getTransformConstraint(std::pair<QVector<double>, QVector<bool> > result)
    {
      QVector3D vec3d((float)result.first[3], (float)result.first[4], (float)result.first[5]);
      QQuaternion qtQuat;
      hpp::Transform__var trans = new hpp::Transform_;
      hpp::boolSeq_var boolSeq = new hpp::boolSeq;

      if (!vec3d.isNull()) {
          qtQuat = QQuaternion::fromAxisAndAngle(vec3d.normalized(), vec3d.length());
          const float theta = (float) vec3d.length();
          qtQuat = QQuaternion(std::cos(theta/2), std::sin(theta/2) * vec3d / theta);
      }

      boolSeq->length(6);
      trans.inout()[0] = result.first[0];
      trans.inout()[1] = result.first[1];
      trans.inout()[2] = result.first[2];
      trans.inout()[3] = qtQuat.scalar();
      trans.inout()[4] = qtQuat.x();
      trans.inout()[5] = qtQuat.y();
      trans.inout()[6] = qtQuat.z();
      boolSeq[0] = result.second[0];
      boolSeq[1] = result.second[1];
      boolSeq[2] = result.second[2];
      boolSeq[3] = result.second[3];
      boolSeq[4] = result.second[4];
      boolSeq[5] = result.second[5];

      plugin_->client()->problem()->
            createTransformationConstraint(to_corba(name_),
                                        to_corba(firstJointName_),
                                        to_corba(secondJointName_),
                                        trans.in(), boolSeq.in());
      emit constraintCreated(name_);
      emit finished();
    }

    void TransformConstraint::operator ()(QString const& name)
    {
      QString firstJoint = (firstJoint_->isEnabled()) ? firstJoint_->currentText() : "";
      QString secondJoint = (secondJoint_->isEnabled()) ? secondJoint_->currentText() : "";

      if (firstJoint == secondJoint) {
        QMessageBox::information(NULL, "hpp-gui", "You have to select two different joints");
        return ;
      }

      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     true, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
                   SLOT(getTransformConstraint(std::pair<QVector<double>,QVector<bool> >)));
      tcw->show();
      name_ = name;
      firstJointName_ = firstJoint;
      secondJointName_ = secondJoint;
    }
  }
}
