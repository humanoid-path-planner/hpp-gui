#include <omniORB4/CORBA.h>
#include "gepetto/gui/meta.hh"

#include <gepetto/viewer/node.h>

#include "iconstraint.hh"
#include "hppwidgetsplugin/transformconstraintwidget.hh"

namespace hpp {
  namespace gui {
    PositionConstraint::PositionConstraint(HppWidgetsPlugin *plugin)
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

    void PositionConstraint::operator ()(QString const& name, QString const& firstJoint,
                                         QString const& secondJoint)
    {
      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     true, false, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
              SLOT(getPositionConstraint(std::pair<QVector<double>,QVector<bool> >)));
      name_ = name;
      firstJoint_ = firstJoint;
      secondJoint_ = secondJoint;
      tcw->show();
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
              createPositionConstraint(gepetto::gui::Traits<QString>::to_corba(name_),
                                       gepetto::gui::Traits<QString>::to_corba(firstJoint_),
                                       gepetto::gui::Traits<QString>::to_corba(secondJoint_),
                                       first.in(), second.in(), boolSeq.in());
        emit finished(name_);
    }

    OrientationConstraint::OrientationConstraint(HppWidgetsPlugin *plugin) {
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
      QVector3D vec3d(result.first[0], result.first[1], result.first[2]);
      QQuaternion qtQuat;
      hpp::Quaternion__var quat = new hpp::Quaternion_;
      hpp::boolSeq_var boolSeq = new hpp::boolSeq;

      if (!vec3d.isNull()) {
          qtQuat = QQuaternion::fromAxisAndAngle(vec3d.normalized(), vec3d.length());
          const double theta = vec3d.length();
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
            createOrientationConstraint(gepetto::gui::Traits<QString>::to_corba(name_),
                                        gepetto::gui::Traits<QString>::to_corba(firstJoint_),
                                        gepetto::gui::Traits<QString>::to_corba(secondJoint_),
                                        quat.in(), boolSeq.in());
      emit finished(name_);
    }

    void OrientationConstraint::operator ()(QString const& name, QString const& firstJoint,
                                            QString const& secondJoint)
    {
      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     false, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
                   SLOT(getOrientationConstraint(std::pair<QVector<double>,QVector<bool> >)));
      tcw->show();
      name_ = name;
      firstJoint_ = firstJoint;
      secondJoint_ = secondJoint;
    }

    TransformConstraint::TransformConstraint(HppWidgetsPlugin *plugin) {
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
      QVector3D vec3d(result.first[3], result.first[4], result.first[5]);
      QQuaternion qtQuat;
      hpp::Transform__var trans = new hpp::Transform_;
      hpp::boolSeq_var boolSeq = new hpp::boolSeq;

      if (!vec3d.isNull()) {
          qtQuat = QQuaternion::fromAxisAndAngle(vec3d.normalized(), vec3d.length());
          const double theta = vec3d.length();
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
            createTransformationConstraint(gepetto::gui::Traits<QString>::to_corba(name_),
                                        gepetto::gui::Traits<QString>::to_corba(firstJoint_),
                                        gepetto::gui::Traits<QString>::to_corba(secondJoint_),
                                        trans.in(), boolSeq.in());
      emit finished(name_);
    }

    void TransformConstraint::operator ()(QString const& name, QString const& firstJoint,
                                            QString const& secondJoint)
    {
      TransformConstraintWidget* tcw = new TransformConstraintWidget(firstJoint, secondJoint,
                                                                     true, true);

      connect(tcw, SIGNAL(finished(std::pair<QVector<double>,QVector<bool> >)),
                   SLOT(getTransformConstraint(std::pair<QVector<double>,QVector<bool> >)));
      tcw->show();
      name_ = name;
      firstJoint_ = firstJoint;
      secondJoint_ = secondJoint;
    }
  }
}
