#include "hpp/gui/tree-item.h"

#include <iostream>

#include <hpp/corbaserver/client.hh>
#include <gepetto/viewer/group-node.h>

#include "hpp/gui/mainwindow.h"

const int JointTreeItem::IndexRole      = Qt::UserRole + 1;
const int JointTreeItem::LowerBoundRole = Qt::UserRole + 2;
const int JointTreeItem::UpperBoundRole = Qt::UserRole + 3;
const int JointTreeItem::NumberDofRole  = Qt::UserRole + 4;
const int JointTreeItem::TypeRole       = Qt::UserRole + 10;

QPushButton* JointItemDelegate::forceIntegrator = 0;

void JointItemDelegate::updateTypeRole (JointTreeItem::ItemType& type)
{
  if (forceIntegrator && forceIntegrator->isChecked () && (
        type == JointTreeItem::UnboundedValueType
        || type == JointTreeItem::BoundedValueType
        ))
    type = JointTreeItem::IntegratorType;
}

BodyTreeItem::BodyTreeItem(graphics::NodePtr_t node) :
  QStandardItem (QString (node->getID().c_str())),
  node_ (node)
{
  init();
  setEditable(false);
}

QStandardItem* BodyTreeItem::clone() const
{
  return new BodyTreeItem (node_);
}

graphics::NodePtr_t BodyTreeItem::node() const
{
  return node_;
}

void BodyTreeItem::init ()
{
  graphics::GroupNodePtr_t gn = boost::dynamic_pointer_cast <graphics::GroupNode> (node_);
  visibility_ = new VisibilityItem (this, Qt::Checked);
  appendRow(visibility_);
  if (gn) {
      for (size_t i = 0; i < gn->getNumOfChildren(); ++i)
        appendRow(new BodyTreeItem (gn->getChild(i)));
    }
}

void VisibilityItem::update()
{
  if (checkState() != state_) {
      state_ = checkState();
      switch (state_) {
        case Qt::Checked:
          parent_->node_->setVisibilityMode(graphics::VISIBILITY_ON);
          break;
        case Qt::Unchecked:
          parent_->node_->setVisibilityMode(graphics::VISIBILITY_OFF);
          break;
        }
  }
}

JointTreeItem::JointTreeItem(const char* name, const hpp::floatSeq &q,
                             const hpp::corbaserver::jointBoundSeq &b,
                             const unsigned int nbDof, graphics::NodePtr_t node)
  : QStandardItem (QString (name)), name_ (name), node_ (node), value_ ()
{
  setData((int)-1, IndexRole);
  setData(nbDof, NumberDofRole);
  setData(SkipType, TypeRole);
  for (size_t i = 0; i < q.length(); ++i) {
      QStandardItem *joint = new QStandardItem;
      QStandardItem *upper = new QStandardItem;
      QStandardItem *lower = new QStandardItem;
      joint->setData(i, IndexRole);
      QList <QStandardItem*> row;
      row << joint << lower << upper;
      value_.append(row);
      appendRow(row);
    }
  updateConfig(q);
  updateBounds(b);
}

QStandardItem *JointTreeItem::clone() const
{
  hpp::floatSeq q = hpp::floatSeq();
  q.length(value_.size());
  hpp::corbaserver::jointBoundSeq b = hpp::corbaserver::jointBoundSeq();
  b.length (2*value_.size());
  for (size_t i = 0; i < q.length(); ++i) {
      q[i] = value_[i][0]->data(Qt::EditRole).toFloat();
      b[2*i] = value_[i][0]->data(LowerBoundRole).toFloat();
      b[2*i+1] = value_[i][0]->data(UpperBoundRole).toFloat();
    }
  return new JointTreeItem (name_.c_str(), q, b, data (NumberDofRole).toInt(), node_);
}

hpp::floatSeq JointTreeItem::config() const
{
  hpp::floatSeq q = hpp::floatSeq();
  q.length(value_.size());
  for (size_t i = 0; i < q.length(); ++i)
      q[i] = value_[i][0]->data(Qt::EditRole).toFloat();
  return q;
}

hpp::corbaserver::jointBoundSeq JointTreeItem::bounds() const
{
  hpp::corbaserver::jointBoundSeq b =
      hpp::corbaserver::jointBoundSeq();
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

void JointTreeItem::updateBounds(const hpp::corbaserver::jointBoundSeq& b)
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
  bool integrate = (data(NumberDofRole).toInt() != value_.size());
  for (int i = 0; i < value_.size(); ++i) {
      float lo = value_[i][1]->data (Qt::EditRole).toFloat();
      float up = value_[i][2]->data (Qt::EditRole).toFloat();
      if (integrate)    value_[i][0]->setData(IntegratorType,     TypeRole);
      else if (lo < up) value_[i][0]->setData(BoundedValueType,   TypeRole);
      else              value_[i][0]->setData(UnboundedValueType, TypeRole);
  }
}

JointItemDelegate::JointItemDelegate(MainWindow *parent)
  : QItemDelegate (parent), main_ (parent)
{}

QWidget *JointItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
            new IntegratorWheel (Qt::Horizontal, parent, main_,
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
            new SliderBoundedJoint (Qt::Horizontal, parent,
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
        hpp::floatSeq_var q = main_->hppClient()->robot()->getJointConfig (ji->name().c_str());
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
      main_->hppClient()->robot()->setJointConfig (ji->name().c_str(), ji->config());
      break;
    case JointTreeItem::BoundType:
      main_->hppClient()->robot()->setJointBounds (ji->name().c_str(), ji->bounds());
      ji->updateTypeRole();
      break;
    default:
      break;
    }
  main_->applyCurrentConfiguration();
}

void JointItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}

IntegratorWheel::IntegratorWheel(Qt::Orientation o, QWidget *parent,
                                 MainWindow *main, std::string jointName,
                                 int nbDof, int index)
  : QSlider (o, parent), rate_ (100), main_ (main), jointName_ (jointName),
    bound_ (100), maxVelocity_ (0.1),
    currentValue_ (0), dq_ (new hpp::floatSeq), nbDof_ (nbDof), index_ (index)
{
  setMinimum(-bound_);
  setMaximum(bound_);
  setValue (0);
  dq_->length (nbDof_);
  for (size_t i = 0; i < dq_->length(); ++i) dq_[i] = 0;
  connect(this, SIGNAL (sliderReleased()), this, SLOT (reset()));
  connect(this, SIGNAL (sliderMoved(int)), this, SLOT (updateIntegrator(int)));
  timerId_ = startTimer(rate_);
}

void IntegratorWheel::timerEvent(QTimerEvent *)
{
  killTimer(timerId_);
  if (currentValue_ != 0) {
      dq_[index_] = currentValue_;
      main_->hppClient()->robot ()->jointIntegrate (jointName_.c_str(), dq_.in());
      main_->applyCurrentConfiguration();
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

SliderBoundedJoint::SliderBoundedJoint(Qt::Orientation orientation, QWidget *parent,
                         MainWindow *main, std::string jointName,
                         hpp::floatSeq *q, int index, double min, double max)
  : QSlider (orientation, parent), main_ (main), jointName_ (jointName),
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
  main_->hppClient()->robot()->setJointConfig (jointName_.c_str(), q_.in());
  main_->applyCurrentConfiguration();
}
