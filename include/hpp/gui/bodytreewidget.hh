#ifndef HPP_GUI_BODYTREEWIDGET_HH
#define HPP_GUI_BODYTREEWIDGET_HH

// This does not work because of qt meta-object compiler
#define HPP_GUI_BODYTREE_DECL_FEATURE(func, ArgType) \
  public slots: \
    void func (ArgType arg)
#define HPP_GUI_BODYTREE_IMPL_FEATURE(func, ArgType, CorbaType, WindowsManagerFunc) \
  void BodyTreeWidget::func (ArgType arg) { \
    WindowsManagerPtr_t wsm = MainWindow::instance()->osg(); \
    foreach (const QModelIndex& index, view_->selectionModel ()->selectedIndexes ()) { \
      const BodyTreeItem *item = dynamic_cast <const BodyTreeItem*> \
        (model_->itemFromIndex (index)); \
      if (item) wsm->WindowsManagerFunc (item->node()->getID().c_str(), \
                                         Traits<CorbaType>::from (arg).in()); \
    } \
  }

#include <QWidget>
#include <QTreeView>
#include <QToolBox>
#include <QStandardItemModel>
#include <QVector3D>

#include <gepetto/viewer/group-node.h>

#include <hpp/gui/fwd.hh>

namespace hpp {
  namespace gui {
    class BodyTreeWidget : public QWidget
    {
      Q_OBJECT

    public:
      explicit BodyTreeWidget (QWidget* parent = NULL)
        : QWidget (parent)
      {}

      void init(QTreeView *view, QToolBox* toolBox);

      virtual ~BodyTreeWidget () {}

      void addBodyToTree (graphics::GroupNodePtr_t group);
      void changeAlphaValue(const float& alpha);

      QTreeView* view ();

    public slots:
      void selectBodyByName (const QString bodyName);
      void selectBodyByName (const std::string& bodyName);
      void reloadBodyTree ();

    protected slots:
      void customContextMenu (const QPoint& pos);

    public slots:
      void setTransparency(int value);
      void setVisibilityMode (QString arg);
      void setWireFrameMode (QString arg);
      void setColor (QColor color);
      void setScale (int scale);

    private:
      QTreeView* view_;
      QStandardItemModel* model_;
      WindowsManagerPtr_t osg_;
      QToolBox* toolBox_;
    };
  }
}

#endif // HPP_GUI_BODYTREEWIDGET_HH
