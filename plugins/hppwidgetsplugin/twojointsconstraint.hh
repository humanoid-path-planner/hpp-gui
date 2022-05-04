//
// Copyright (c) CNRS
// Author: Heidy Dallard
//

#ifndef HPP_GUI_TWOJOINTSCONSTRAINT_HH
#define HPP_GUI_TWOJOINTSCONSTRAINT_HH

#include "hppwidgetsplugin/hppwidgetsplugin.hh"
#include "iconstraint.hh"

class QCheckBox;
class QComboBox;
class QString;
class QWidget;

namespace hpp {
namespace gui {
class ATwoJointConstraint : public IConstraint {
  Q_OBJECT

 protected slots:
  void firstJointSelect(int index);
  void globalSelected(bool action);

 public:
  virtual ~ATwoJointConstraint();

  virtual QWidget* getWidget() const;
  virtual void reload();

 protected:
  ATwoJointConstraint(HppWidgetsPlugin* plugin);

  QWidget* widget_;
  QComboBox* firstJoint_;
  QComboBox* secondJoint_;
  QCheckBox* globalFirst_;
  QCheckBox* globalSecond_;

  HppWidgetsPlugin* plugin_;
  hpp::Names_t_var joints_;

  QString name_;
  QString firstJointName_;
  QString secondJointName_;
};

class PositionConstraint : public ATwoJointConstraint {
  Q_OBJECT

 private slots:
  void getPositionConstraint(std::pair<QVector<double>, QVector<bool> > result);

 public:
  explicit PositionConstraint(HppWidgetsPlugin* plugin);
  ~PositionConstraint();

  virtual QString getName() const;
  virtual void operator()(QString const& name);

 private:
  HppWidgetsPlugin* plugin_;
};

class OrientationConstraint : public ATwoJointConstraint {
  Q_OBJECT

 private slots:
  void getOrientationConstraint(
      std::pair<QVector<double>, QVector<bool> > result);

 public:
  explicit OrientationConstraint(HppWidgetsPlugin* plugin);
  ~OrientationConstraint();

  QString getName() const;
  void operator()(QString const& name);

 private:
  HppWidgetsPlugin* plugin_;
};

class TransformConstraint : public ATwoJointConstraint {
  Q_OBJECT

 private slots:
  void getTransformConstraint(
      std::pair<QVector<double>, QVector<bool> > result);

 public:
  explicit TransformConstraint(HppWidgetsPlugin* plugin);
  ~TransformConstraint();

  QString getName() const;
  void operator()(QString const& name);

 private:
  HppWidgetsPlugin* plugin_;
  QString name_;
};
}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_TWOJOINTSCONSTRAINT_HH
