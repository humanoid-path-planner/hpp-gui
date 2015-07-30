#ifndef TREEITEM_H
#define TREEITEM_H

#include <QStandardItem>
#include <QItemDelegate>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QPushButton>

#include <hpp/gui/fwd.h>
#include <gepetto/viewer/node.h>

class BodyTreeItem;

class VisibilityItem :public QStandardItem
{
public:
  VisibilityItem (BodyTreeItem* parent, Qt::CheckState state = Qt::Checked) :
    QStandardItem ("Visible"),
    state_ (state),
    parent_ (parent)
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

  VisibilityItem* visibility_;

  friend class VisibilityItem;
};

#endif // TREEITEM_H
