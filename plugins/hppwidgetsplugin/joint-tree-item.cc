//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#include <omniORB4/CORBA.h>
#include "hppwidgetsplugin/joint-tree-item.hh"

#include <iostream>

#include <hpp/corbaserver/client.hh>
#include <gepetto/viewer/group-node.h>

#include "gepetto/gui/mainwindow.hh"

#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppwidgetsplugin/pathplayer.hh"
#include "hppwidgetsplugin/joint-action.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
    const int JointTreeItem::IndexRole      = Qt::UserRole + 1;
    const int JointTreeItem::LowerBoundRole = Qt::UserRole + 2;
    const int JointTreeItem::UpperBoundRole = Qt::UserRole + 3;
    const int JointTreeItem::NumberDofRole  = Qt::UserRole + 4;
    const int JointTreeItem::TypeRole       = Qt::UserRole + 10;

    JointTreeItem::JointTreeItem(const char* name, const hpp::floatSeq &q,
        const hpp::floatSeq &b,
        const unsigned int nbDof, const NodesPtr_t& nodes)
      : QStandardItem (QString (name)), name_ (name), nodes_ (nodes), value_ ()
    {
      setData((int)-1, IndexRole);
      setData(nbDof, NumberDofRole);
      setData(SkipType, TypeRole);
      for (size_t i = 0; i < q.length(); ++i) {
        QStandardItem *joint = new QStandardItem;
        QStandardItem *upper = new QStandardItem;
        QStandardItem *lower = new QStandardItem;
        joint->setData(static_cast<int>(i), IndexRole);
        QList <QStandardItem*> row;
        row << joint << lower << upper;
        value_.append(row);
        appendRow(row);
      }
      updateConfig(q);
      updateBounds(b);
    }

    JointTreeItem::~JointTreeItem ()
    {
      qDeleteAll(actions_);
    }

    QStandardItem *JointTreeItem::clone() const
    {
      hpp::floatSeq q = hpp::floatSeq();
      q.length(value_.size());
      hpp::floatSeq b = hpp::floatSeq();
      b.length (2*value_.size());
      for (size_t i = 0; i < q.length(); ++i) {
        q[(ULong) i] = value_[(ULong) i][0]->data(Qt::EditRole).toFloat();
        b[2*(ULong) i] = value_[(ULong) i][0]->data(LowerBoundRole).toFloat();
        b[2*(ULong) i+1] = value_[(ULong) i][0]->data(UpperBoundRole).toFloat();
      }
      return new JointTreeItem (name_.c_str(), q, b, data (NumberDofRole).toInt(), nodes_);
    }

    hpp::floatSeq JointTreeItem::config() const
    {
      hpp::floatSeq q = hpp::floatSeq();
      q.length(value_.size());
      for (size_t i = 0; i < q.length(); ++i)
        q[(ULong) i] = value_[(ULong) i][0]->data(Qt::EditRole).toFloat();
      return q;
    }

    hpp::floatSeq JointTreeItem::bounds() const
    {
      hpp::floatSeq b = hpp::floatSeq();
      b.length (2*value_.size());
      for (int i = 0; i < value_.size(); ++i) {
        b[2*i  ] = value_[i][1]->data(Qt::EditRole).toFloat();
        b[2*i+1] = value_[i][2]->data(Qt::EditRole).toFloat();
      }
      return b;
    }

    void JointTreeItem::updateConfig (const hpp::floatSeq& c)
    {
      assert ((int)c.length() == value_.size());
      for (int i = 0; i < value_.size(); ++i)
        value_[i][0]->setData(c[i], Qt::EditRole);
    }

    void JointTreeItem::updateBounds(const hpp::floatSeq& b)
    {
      assert ((int)b.length() == 2*value_.size());
      for (int i = 0; i < value_.size(); ++i) {
        QStandardItem *lower = value_[i][1];
        QStandardItem *upper = value_[i][2];
        lower->setData(BoundType, TypeRole);
        upper->setData(BoundType, TypeRole);
        lower->setData(b[2*i  ], Qt::EditRole);
        upper->setData(b[2*i+1], Qt::EditRole);
      }
      updateTypeRole();
    }

    void JointTreeItem::updateTypeRole()
    {
      // planar and freeflyer joints
      int threshold = value_.size();
      // TODO SO3 joint fall in that case too whereas threshold should be 0 for them.
      if (data(NumberDofRole).toInt() == 3 && value_.size() == 4 )
        threshold = 2;
      else if (data(NumberDofRole).toInt() == 6 && value_.size() == 7 )
        threshold = 3;
      for (int i = 0; i < value_.size(); ++i) {
        float lo = value_[i][1]->data (Qt::EditRole).toFloat();
        float up = value_[i][2]->data (Qt::EditRole).toFloat();
        if (i >= threshold) value_[i][0]->setData(IntegratorType,     TypeRole);
        else if (lo < up)   value_[i][0]->setData(BoundedValueType,   TypeRole);
        else                value_[i][0]->setData(UnboundedValueType, TypeRole);
      }
    }

    void JointTreeItem::setupActions (HppWidgetsPlugin* plugin)
    {
      JointAction* a;

      a = new JointAction (QObject::tr("Move &joint..."), name_, 0);
      plugin->jointTreeWidget()->connect (a, SIGNAL (triggered(std::string)),
          SLOT (openJointMoveDialog(std::string)));
      actions_.append(a);

      a = new JointAction (QObject::tr("Set &bounds..."), name_, 0);
      plugin->jointTreeWidget()->connect (a, SIGNAL (triggered(std::string)),
          SLOT (openJointBoundDialog(std::string)));
      actions_.append(a);

      a = new JointAction (QObject::tr("Add joint &frame"), name_, 0);
      plugin->connect (a, SIGNAL (triggered(std::string)),
          SLOT (addJointFrame(std::string)));
      actions_.append(a);

      a = new JointAction (QObject::tr("Display &roadmap"), name_, 0);
      plugin->connect (a, SIGNAL (triggered(std::string)),
          SLOT (displayRoadmap(std::string)));
      actions_.append(a);

      a = new JointAction (QObject::tr("Display &waypoints of selected path"), name_, 0);
      plugin->pathPlayer()->connect (a, SIGNAL (triggered(std::string)),
          SLOT (displayWaypointsOfPath(std::string)));
      actions_.append(a);

      a = new JointAction (QObject::tr("Display selected &path"), name_, 0);
      plugin->pathPlayer()->connect (a, SIGNAL (triggered(std::string)),
          SLOT (displayPath(std::string)));
      actions_.append(a);
    }

    const QList<QAction*>& JointTreeItem::actions () const
    {
      return actions_;
    }

    JointItemDelegate::JointItemDelegate(QPushButton *forceVelocity, HppWidgetsPlugin *plugin, gepetto::gui::MainWindow *parent)
      : QItemDelegate (parent), main_ (parent),
      plugin_ (plugin),
      forceIntegrator_ (forceVelocity)
    {}

    void JointItemDelegate::updateTypeRole (JointTreeItem::ItemType& type) const
    {
      if (forceIntegrator_ && forceIntegrator_->isChecked () && (
            type == JointTreeItem::UnboundedValueType
            || type == JointTreeItem::BoundedValueType
            ))
        type = JointTreeItem::IntegratorType;
    }

    QWidget *JointItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
    {
      JointTreeItem::ItemType type = (JointTreeItem::ItemType)index.data(JointTreeItem::TypeRole).toInt();
      updateTypeRole(type);
      const QStandardItemModel* m = static_cast <const QStandardItemModel*> (index.model());
      const JointTreeItem* ji = dynamic_cast <const JointTreeItem*> (m->itemFromIndex(index)->parent());
      switch (type) {
        case JointTreeItem::SkipType:
          return 0;
        case JointTreeItem::IntegratorType: {
                                              assert (ji);
                                              IntegratorWheel* wheel =
                                                new IntegratorWheel (Qt::Horizontal, plugin_, parent, main_,
                                                    ji->name(),
                                                    ji->data(JointTreeItem::NumberDofRole).toInt(),
                                                    std::min (
                                                      index.data(JointTreeItem::IndexRole).toInt(),
                                                      ji->data(JointTreeItem::NumberDofRole).toInt()-1));
                                              return wheel;
                                            }
        case JointTreeItem::BoundedValueType: {
                                                assert (ji);
                                                hpp::floatSeq* q = new hpp::floatSeq;
                                                hpp::floatSeq cfg = ji->config();
                                                q->length(cfg.length());
                                                *q = cfg;
                                                const QModelIndex& lower = index.sibling(index.row(), 1);
                                                const QModelIndex& upper = index.sibling(index.row(), 2);
                                                float lo = lower.data(Qt::EditRole).toFloat(),
                                                      up = upper.data(Qt::EditRole).toFloat();
                                                SliderBoundedJoint* slider =
                                                  new SliderBoundedJoint (Qt::Horizontal, plugin_, parent,
                                                      main_, ji->name(), q, index.data(JointTreeItem::IndexRole).toInt(),
                                                      lo, up);
                                                return slider;
                                              }
        case JointTreeItem::UnboundedValueType:
        case JointTreeItem::BoundType: {
                                         QDoubleSpinBox* spinbox = new QDoubleSpinBox (parent);
                                         spinbox->setMinimum(-DBL_MAX);
                                         spinbox->setMaximum(DBL_MAX);
                                         spinbox->setSingleStep(0.01);
                                         spinbox->setDecimals(3);
                                         return spinbox;
                                       }
        default:
                                       return 0;
      }
    }

    void JointItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
      JointTreeItem::ItemType type = (JointTreeItem::ItemType)index.data(JointTreeItem::TypeRole).toInt();
      updateTypeRole(type);
      float q = index.data(Qt::EditRole).toFloat ();
      switch (type) {
        case JointTreeItem::SkipType:
          return;
        case JointTreeItem::IntegratorType:
          return;
        case JointTreeItem::BoundedValueType:
          return;
        case JointTreeItem::UnboundedValueType:
        case JointTreeItem::BoundType:{
                                        QDoubleSpinBox* spinbox = static_cast <QDoubleSpinBox*> (editor);
                                        spinbox->setValue(q);
                                        break;
                                      }
        default:
                                      break;
      }
    }

    void JointItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
      JointTreeItem::ItemType type = (JointTreeItem::ItemType)index.data(JointTreeItem::TypeRole).toInt();
      updateTypeRole(type);
      QStandardItemModel* m = static_cast <QStandardItemModel*> (model);
      JointTreeItem* ji = dynamic_cast <JointTreeItem*> (m->itemFromIndex(index)->parent());
      double q;
      switch (type) {
        case JointTreeItem::SkipType:
          return;
        case JointTreeItem::IntegratorType:{
                                             hpp::floatSeq_var q = plugin_->client()->robot()->getJointConfig (ji->name().c_str());
                                             ji->updateConfig(q.in());
                                             return;
                                           }
        case JointTreeItem::BoundedValueType: {
                                                SliderBoundedJoint* slider = static_cast <SliderBoundedJoint*> (editor);
                                                q = slider->getValue();
                                                break;
                                              }
        case JointTreeItem::UnboundedValueType:
        case JointTreeItem::BoundType: {
                                         QDoubleSpinBox* spinbox = static_cast <QDoubleSpinBox*> (editor);
                                         q = spinbox->value();
                                         break;
                                       }
        default:
                                       return;
      }
      model->setData(index, q, Qt::EditRole);
      assert (ji);
      switch (type) {
        case JointTreeItem::BoundedValueType:
          return;
        case JointTreeItem::UnboundedValueType:
          plugin_->client()->robot()->setJointConfig (ji->name().c_str(), ji->config());
          break;
        case JointTreeItem::BoundType:
          plugin_->client()->robot()->setJointBounds (ji->name().c_str(), ji->bounds());
          ji->updateTypeRole();
          break;
        default:
          break;
      }
      main_->requestApplyCurrentConfiguration();
    }

    void JointItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
    {
      editor->setGeometry(option.rect);
    }

    IntegratorWheel::IntegratorWheel(Qt::Orientation o, HppWidgetsPlugin *plugin, QWidget *parent,
        gepetto::gui::MainWindow *main, std::string jointName,
        int nbDof, int index)
      : QSlider (o, parent), rate_ (100), main_ (main), plugin_ (plugin), jointName_ (jointName),
      bound_ (100), maxVelocity_ (0.1),
      currentValue_ (0), dq_ (new hpp::floatSeq), nbDof_ (nbDof), index_ (index)
    {
      setMinimum(-bound_);
      setMaximum(bound_);
      setValue (0);
      dq_->length (nbDof_);
      for (size_t i = 0; i < dq_->length(); ++i) dq_[(ULong) i] = 0;
      connect(this, SIGNAL (sliderReleased()), this, SLOT (reset()));
      connect(this, SIGNAL (sliderMoved(int)), this, SLOT (updateIntegrator(int)));
      timerId_ = startTimer(rate_);
    }

    void IntegratorWheel::timerEvent(QTimerEvent *)
    {
      killTimer(timerId_);
      if (currentValue_ != 0) {
        dq_[index_] = currentValue_;
        plugin_->client()->robot ()->jointIntegrate (jointName_.c_str(), dq_.in());
        main_->requestApplyCurrentConfiguration();
      }
      timerId_ = startTimer(rate_);
    }

    void IntegratorWheel::reset()
    {
      currentValue_ = 0;
      setValue(0);
    }

    void IntegratorWheel::updateIntegrator(int value)
    {
      currentValue_ = maxVelocity_ * (double)value / (double)bound_;
    }

    SliderBoundedJoint::SliderBoundedJoint(Qt::Orientation orientation, HppWidgetsPlugin *plugin, QWidget *parent,
        gepetto::gui::MainWindow *main, std::string jointName,
        hpp::floatSeq *q, int index, double min, double max)
      : QSlider (orientation, parent), main_ (main), plugin_ (plugin), jointName_ (jointName),
      q_ (q), index_ (index), m_ (min), M_ (max)
    {
      setMinimum(0);
      setMaximum(100);
      setValue ((int)(100*(q_[index_] - m_)/(M_ - m_)));
      connect (this, SIGNAL (sliderMoved(int)), this, SLOT (updateConfig(int)));
    }

    double SliderBoundedJoint::getValue()
    {
      return q_[index_];
    }

    void SliderBoundedJoint::updateConfig(int value)
    {
      q_[index_] = m_ + (double)(value - 0) * (M_ - m_) / (double)100;
      plugin_->client()->robot()->setJointConfig (jointName_.c_str(), q_.in());
      main_->requestApplyCurrentConfiguration();
    }
  } // namespace gui
} // namespace hpp
