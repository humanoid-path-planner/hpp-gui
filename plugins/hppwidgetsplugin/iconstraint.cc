#include <omniORB4/CORBA.h>
#include "hpp/gui/meta.hh"

#include "iconstraint.hh"
#include "hppwidgetsplugin/positionconstraintwidget.hh"

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
        PositionConstraintWidget* psw = new PositionConstraintWidget(firstJoint, secondJoint);

        name_ = name;
        firstJoint_ = firstJoint;
        secondJoint_ = secondJoint;
        connect(psw, SIGNAL(values(QVector<double>)), SLOT(getPositionConstraint(QVector<double>)));
        psw->exec();
    }

    void PositionConstraint::getPositionConstraint(QVector<double> vec)
    {
        hpp::floatSeq_var first = new hpp::floatSeq;
        hpp::floatSeq_var second = new hpp::floatSeq;
        hpp::boolSeq_var boolSeq = new hpp::boolSeq;

        first->length(3);
        second->length(3);
        boolSeq->length(3);
        first[0] = vec[0];
        first[1] = vec[1];
        first[2] = vec[2];
        second[0] = vec[3];
        second[1] = vec[4];
        second[2] = vec[5];
        boolSeq[0] = true;
        boolSeq[1] = false;
        boolSeq[2] = false;

        plugin_->client()->problem()->
              createPositionConstraint(Traits<QString>::to_corba(name_),
                                       Traits<QString>::to_corba(firstJoint_),
                                       Traits<QString>::to_corba(secondJoint_),
                                       first.in(), second.in(), boolSeq.in());
        emit finished(name_);
    }
  }
}
