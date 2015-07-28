#include "hpp/gui/tree-item.h"

#include <QDebug>

#include <gepetto/viewer/group-node.h>

#include "hpp/gui/mainwindow.h"

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
        case Qt::PartiallyChecked:
          qDebug () << "Not implemented";
          break;
        }
  }
}
