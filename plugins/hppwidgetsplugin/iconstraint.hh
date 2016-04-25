#ifndef ICONSTRAINT_HH
#define ICONSTRAINT_HH

#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    class IConstraint : public QObject
    {
        Q_OBJECT
    signals:
      void finished(QString name);

    public:
      virtual ~IConstraint() {}
      virtual QString getName() const = 0;
      virtual void operator()(QString const& name, QString const& firstJoint,
                              QString const& secondJoint) = 0;
    };

    class PositionConstraint : public IConstraint
    {
        Q_OBJECT

    private slots:
      void getPositionConstraint(QVector<double> vec);

    public:
      explicit PositionConstraint(HppWidgetsPlugin* plugin);
      ~PositionConstraint();

      QString getName() const;
      void operator()(QString const& name, QString const& firstJoint,
                      QString const& secondJoint);

    private:
      HppWidgetsPlugin* plugin_;
      QString name_;
      QString firstJoint_;
      QString secondJoint_;
    };
  }
}
#endif // ICONSTRAINT_HH
