#include <hpp/gui/bodytreewidget.hh>

#include <hpp/gui/tree-item.hh>
#include <hpp/gui/mainwindow.hh>
#include <hpp/gui/windows-manager.hh>
#include <hpp/gui/osgwidget.hh>
#include <hpp/gui/meta.hh>

#include <QSignalMapper>

static void addSelector (QToolBox* tb, QString title, QStringList display, QStringList command,
                         QObject* receiver, const char* slot) {
  QWidget* newW = new QWidget();
  newW->setObjectName(title);
  QSignalMapper* mapper = new QSignalMapper (tb);
  QHBoxLayout* layout = new QHBoxLayout(newW);
  layout->setSpacing(6);
  layout->setContentsMargins(11, 11, 11, 11);
  layout->setObjectName(title + "_layout");
  for (int i = 0; i < display.size(); ++i) {
      QPushButton* button = new QPushButton(display[i], newW);
      button->setObjectName(title + "_button_" + display[i]);
      layout->addWidget (button);
      mapper->setMapping(button, command[i]);
      QObject::connect (button, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    }
  QObject::connect (mapper, SIGNAL(mapped(QString)), receiver, slot);
  tb->addItem(newW, title);
}

namespace hpp {
  namespace gui {
    void BodyTreeWidget::init(QTreeView* view, QToolBox *toolBox)
    {
      MainWindow* main = MainWindow::instance();
      osg_ = main->osg();
      view_ = view;
      toolBox_ = toolBox;
      model_  = new QStandardItemModel;
      view_->setModel(model_);
      view_->setSelectionMode(QAbstractItemView::SingleSelection);

      connect (main, SIGNAL (refresh()), SLOT (reloadBodyTree()));
      connect (view_->selectionModel(), SIGNAL (selectionChanged(QItemSelection,QItemSelection)),
          SLOT (bodySelectionChanged(QItemSelection,QItemSelection)));
      connect (view_, SIGNAL (customContextMenuRequested(QPoint)), SLOT(customContextMenu(QPoint)));

      addSelector (toolBox_, "Visibility",
                   QStringList () << "On" << "Always on top" << "Off",
                   QStringList () << "ON" << "ALWAYS_ON_TOP" << "OFF",
                   this, SLOT(setVisibilityMode(QString)));
      addSelector (toolBox_, "Wireframe mode",
                   QStringList () << "Fill" << "Both" << "Wireframe",
                   QStringList () << "FILL" << "FILL_AND_WIREFRAME" << "WIREFRAME",
                   this, SLOT(setWireFrameMode(QString)));
    }

    void BodyTreeWidget::selectBodyByName(const QString &bodyName)
    {
      QList<QStandardItem*> matches;
      if (!bodyName.isEmpty() && !bodyName.isNull()) {
        matches = model_->findItems(bodyName, Qt::MatchFixedString
            | Qt::MatchCaseSensitive
            | Qt::MatchRecursive);
      }
      if (matches.empty()) {
        qDebug () << "Body" << bodyName << "not found.";
        view_->clearSelection();
      } else {
        view_->setCurrentIndex(matches.first()->index());
      }
    }

    void BodyTreeWidget::reloadBodyTree()
    {
      model_->clear();
      std::vector <std::string> sceneNames = osg_->getSceneList ();
      for (unsigned int i = 0; i < sceneNames.size(); ++i) {
        graphics::GroupNodePtr_t group = osg_->getScene(sceneNames[i]);
        if (!group) continue;
        addBodyToTree(group);
      }
    }

    void BodyTreeWidget::addBodyToTree(graphics::GroupNodePtr_t group)
    {
      model_->appendRow(new BodyTreeItem (group));
    }

    void BodyTreeWidget::bodySelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
      QStringList sel, desel;
      foreach (QModelIndex index, selected.indexes()) {
        BodyTreeItem *item = dynamic_cast <BodyTreeItem*>
          (model_->itemFromIndex(index));
        if (item) {
          const std::string& s = item->node()->getID();
          sel << QString::fromStdString(s);
          osg_->setHighlight(s.c_str(), 2);
        }
      }
      foreach (QModelIndex index, deselected.indexes()) {
        BodyTreeItem *item = dynamic_cast <BodyTreeItem*>
          (model_->itemFromIndex(index));
        if (item) {
          const std::string& s = item->node()->getID();
          desel << QString::fromStdString(s);
          osg_->setHighlight(s.c_str(), 0);
        }
      }
      emit selectedBodyChanged (sel, desel);
    }

    void BodyTreeWidget::customContextMenu(const QPoint &pos)
    {
      QMenu contextMenu (tr("Node"), this);
      QModelIndex index = view_->indexAt(pos);
      if(index.isValid()) {
          BodyTreeItem *item = dynamic_cast <BodyTreeItem*>
              (model_->itemFromIndex(index));
          if (!item) return;
          MainWindow* main = MainWindow::instance ();
          item->populateContextMenu (&contextMenu);
          QMenu* windows = contextMenu.addMenu(tr("Attach to window"));
          foreach (OSGWidget* w, main->osgWindows ()) {
              QAction* aw = windows->addAction(w->objectName());
              aw->setUserData(0, (QObjectUserData*)w);
            }
          QAction* toDo = contextMenu.exec(view_->mapToGlobal(pos));
          if (!toDo) return;
          if (toDo->parent() == windows) {
              OSGWidget* w = (OSGWidget*)toDo->userData(0);
              if (!w) return;
              item->attachToWindow(w->windowID());
        }
        return;
      }
    }

    HPP_GUI_BODYTREE_IMPL_FEATURE (setVisibilityMode, QString, setVisibility)
    HPP_GUI_BODYTREE_IMPL_FEATURE (setWireFrameMode, QString, setWireFrameMode)
//      HPP_GUI_BODYTREE_IMPL_FEATURE (setAlpha, QString, setAlpha)
  }
}
