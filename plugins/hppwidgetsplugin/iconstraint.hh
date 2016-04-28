#ifndef HPP_GUI_ICONSTRAINT_HH
#define HPP_GUI_ICONSTRAINT_HH

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
      void getPositionConstraint(std::pair<QVector<double>, QVector<bool> > result);

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

    class OrientationConstraint : public IConstraint
    {
      Q_OBJECT

    private slots:
      void getOrientationConstraint(std::pair<QVector<double>, QVector<bool> > result);

    public:
      explicit OrientationConstraint(HppWidgetsPlugin* plugin);
      ~OrientationConstraint();

      QString getName() const;
      void operator()(QString const& name, QString const& firstJoint,
                      QString const& secondJoint);

    private:
      HppWidgetsPlugin* plugin_;
      QString name_;
      QString firstJoint_;
      QString secondJoint_;
    };

    class TransformConstraint : public IConstraint
    {
      Q_OBJECT

    private slots:
      void getTransformConstraint(std::pair<QVector<double>, QVector<bool> > result);

    public:
      explicit TransformConstraint(HppWidgetsPlugin* plugin);
      ~TransformConstraint();

      QString getName() const;
      void operator()(QString const& name, QString const& firstJoint,
                      QString const& secondJoint);

    private:
      HppWidgetsPlugin* plugin_;
      QString name_;
      QString firstJoint_;
      QString secondJoint_;
    };
  } // namespace hpp
} // namespace gui

#endif // HPP_GUI_ICONSTRAINT_HH
