#ifndef HPP_GUI_TREEITEM_HH
#define HPP_GUI_TREEITEM_HH

#include <QStandardItem>
#include <QItemDelegate>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>

#include <hpp/gui/fwd.hh>
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

class BodyTreeItem : public QObject, public QStandardItem
{
  Q_OBJECT

public:
  BodyTreeItem (graphics::NodePtr_t node);

  virtual QStandardItem* clone () const;

  virtual int type() {
    return QStandardItem::UserType+1;
  }

  graphics::NodePtr_t node () const;

  void populateContextMenu (QMenu* menu);

  void setParentGroup (const std::string& parent);

public:
  void attachToWindow (unsigned int windowID);

protected:
  void init ();

protected slots:
  void setViewingMode (QString mode);
  void setVisibilityMode (QString mode);
  void removeFromGroup ();
  void remove ();
  void addLandmark ();
  void deleteLandmark ();

private:
  graphics::NodePtr_t node_;
  std::string parentGroup_;

  VisibilityItem* visibility_;

  QSignalMapper vmMapper_;
  QSignalMapper vizMapper_;

  friend class VisibilityItem;
};

#endif // HPP_GUI_TREEITEM_HH
