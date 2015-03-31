#ifndef TREEITEM_H
#define TREEITEM_H

#include <QStandardItem>
#include <QItemDelegate>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QPushButton>

#include <hpp/corbaserver/robot.hh>
#include <hpp/gui/fwd.h>
#include <gepetto/viewer/node.h>

class BodyTreeItem;

class VisibilityItem :public QStandardItem
{
public:
  VisibilityItem (BodyTreeItem* parent, Qt::CheckState state = Qt::Checked) :
    QStandardItem ("Visible"),
    parent_ (parent),
    state_ (state)
  {
    setEditable(false);
    setCheckable(true);
    setCheckState(state_);
  }

  virtual int type() {
    return QStandardItem::UserType+2;
  }

  virtual QStandardItem* clone () const
  {
    return new VisibilityItem (parent_);
  }

  void update ();

private:
  Qt::CheckState state_;
  BodyTreeItem* parent_;
};

class BodyTreeItem : public QStandardItem
{
public:
  BodyTreeItem (graphics::NodePtr_t node);

  virtual QStandardItem* clone () const;

  virtual int type() {
    return QStandardItem::UserType+1;
  }

  graphics::NodePtr_t node () const;

protected:
  void init ();

private:
  graphics::NodePtr_t node_;
  std::string jointName_, linkName_;

  VisibilityItem* visibility_;

  friend class VisibilityItem;
};

class IntegratorWheel : public QSlider
{
  Q_OBJECT

public:
  IntegratorWheel (Qt::Orientation o, QWidget *parent,
                   MainWindow *main, std::string jointName,
                   int nbDof, int index);

protected:
  void timerEvent(QTimerEvent *);

protected slots:
  void reset ();
  void updateIntegrator (int value);

private:
  int rate_; // in millisecond
  int timerId_;
  MainWindow* main_;

  std::string jointName_;
  const int bound_;
  const double maxVelocity_;
  double currentValue_;
  hpp::floatSeq_var dq_;
  int nbDof_, index_;
};

class SliderBoundedJoint : public QSlider
{
  Q_OBJECT

public:
  SliderBoundedJoint (Qt::Orientation orientation, QWidget* parent,
               MainWindow *main, std::string jointName,
               hpp::floatSeq* q, int index, double min, double max);

  double getValue ();

private slots:
  void updateConfig (int value);

private:
  MainWindow* main_;
  std::string jointName_;
  hpp::floatSeq_var q_;
  int index_;
  double m_, M_;
};

class JointTreeItem : public QStandardItem
{
public:
  static const int IndexRole     ;
  static const int NumberDofRole ;
  static const int LowerBoundRole;
  static const int UpperBoundRole;
  static const int TypeRole      ;

  enum ItemType {
    SkipType = 0,
    IntegratorType,
    UnboundedValueType,
    BoundedValueType,
    BoundType
  };

  JointTreeItem (const char* name, const hpp::floatSeq& q,
                 const hpp::corbaserver::jointBoundSeq& b, const unsigned int nbDof, graphics::NodePtr_t node);

  virtual QStandardItem* clone () const;

  virtual int type() {
    return QStandardItem::UserType+3;
  }

  const std::string& name () const {
    return name_;
  }

  hpp::floatSeq config () const;

  hpp::corbaserver::jointBoundSeq bounds () const;

  void updateBounds (const hpp::corbaserver::jointBoundSeq &b);

  void updateConfig (const hpp::floatSeq &c);

  void updateTypeRole ();

private:
  typedef QList<QStandardItem*> StandardItemList;

  std::string name_;
  graphics::NodePtr_t node_;
  QVector<StandardItemList> value_;
};

class JointItemDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  static QPushButton* forceIntegrator;
  static void updateTypeRole (JointTreeItem::ItemType& type);

  JointItemDelegate (MainWindow* parent);

  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
  MainWindow* main_;
};

#endif // TREEITEM_H
