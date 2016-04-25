#include <omniORB4/CORBA.h>
#include "hpp/gui/meta.hh"

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
      secondJoint_ = secondJoint_;
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
              createPositionConstraint(Traits<QString>::to_corba(name_),
                                       Traits<QString>::to_corba(firstJoint_),
                                       Traits<QString>::to_corba(secondJoint_),
                                       first.in(), second.in(), boolSeq.in());
        emit finished(name_);
    }
  }
}
