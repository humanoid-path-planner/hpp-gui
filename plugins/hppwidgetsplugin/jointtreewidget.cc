//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include <omniORB4/CORBA.h>

#include <QMenu>

#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppwidgetsplugin/ui_jointtreewidget.h"

#include <gepetto/viewer/group-node.h>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/bodytreewidget.hh>
#include <gepetto/gui/windows-manager.hh>
#include <gepetto/gui/action-search-bar.hh>
#if GEPETTO_GUI_HAS_PYTHONQT
#include <gepetto/gui/pythonwidget.hh>
#endif

#include "hppwidgetsplugin/joint-tree-item.hh"
#include "hppwidgetsplugin/jointbounddialog.hh"
#include "hppwidgetsplugin/transformwidget.hh"
#include "hppwidgetsplugin/joint-action.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
    using gepetto::gui::MainWindow;
    using gepetto::gui::ActionSearchBar;

    JointTreeWidget::JointTreeWidget(HppWidgetsPlugin *plugin, QWidget *parent) :
      QWidget(parent),
      plugin_ (plugin),
      ui_ (new ::Ui::JointTreeWidget),
      model_ (new QStandardItemModel),
      dock_ (NULL)
    {
      ui_->setupUi (this);
      ui_->jointTree->setModel(model_);
      ui_->jointTree->setItemDelegate (
          new JointItemDelegate(ui_->button_forceVelocity,
            plugin_,
            gepetto::gui::MainWindow::instance()));
      reset ();
      initSearchActions ();

      connect(ui_->jointTree, SIGNAL (customContextMenuRequested(QPoint)),
          SLOT (customContextMenu(QPoint)));
      connect(ui_->jointTree, SIGNAL (expanded(QModelIndex)),
          SLOT (resize(QModelIndex)));
      connect(ui_->jointTree->selectionModel(), SIGNAL (currentChanged(QModelIndex, QModelIndex)),
          SLOT (currentJointChanged(QModelIndex,QModelIndex)));
    }

    JointTreeWidget::~JointTreeWidget()
    {
      delete ui_;
    }

    void JointTreeWidget::initSearchActions()
    {
      ActionSearchBar* asb = MainWindow::instance()->actionSearchBar();
      JointAction* a;

      a = new JointAction (tr("Move &joint..."), this, this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (openJointMoveDialog(std::string)));
      asb->addAction(a);

      a = new JointAction (tr("Set &bounds..."), this, this);
      connect (a, SIGNAL (triggered(std::string)), SLOT (openJointBoundDialog(std::string)));
      asb->addAction(a);
    }

    void JointTreeWidget::dockWidget(QDockWidget *dock)
    {
      dock_ = dock;
    }

    std::string JointTreeWidget::selectedJoint() const
    {
      QItemSelectionModel* sm = ui_->jointTree->selectionModel();
      JointTreeItem *item = NULL;
      if (sm->currentIndex().isValid()) {
        item = dynamic_cast <JointTreeItem*>
          (model_->itemFromIndex(sm->currentIndex()));
        if (item != NULL)
          return item->name();
      }
      return std::string ();
    }

    QString JointTreeWidget::getSelectedJoint() const
    {
      return QString::fromStdString(selectedJoint());
    }

    void JointTreeWidget::customContextMenu(const QPoint &pos)
    {
      QModelIndex index = ui_->jointTree->indexAt(pos);
      if(index.isValid()) {
        QMenu contextMenu (tr("Node"), this);
        JointTreeItem *item =
          dynamic_cast <JointTreeItem*> (model_->itemFromIndex(index));
        if (!item) return;
        contextMenu.addActions (item->actions());
        MainWindow* main = MainWindow::instance();
        foreach (gepetto::gui::JointModifierInterface* adi,
            main->pluginManager ()->get<gepetto::gui::JointModifierInterface> ()) {
          if (!adi) continue;
          contextMenu.addAction (adi->action (item->name()));
        }
#if GEPETTO_GUI_HAS_PYTHONQT
        QVariantList jass = main->pythonWidget()->callPluginMethod
          ("getJointActions", QVariantList() << item->text());
        foreach (QVariant jas, jass)
        {
          if (!jas.canConvert(QVariant::List)) {
            qDebug () << "Could not convert to QVariantList" << jas;
            continue;
          }
          foreach (QVariant ja, jas.toList()) {
            QAction* action = qobject_cast<QAction*>(ja.value<QObject*>());
            if (action) contextMenu.addAction (action);
            else {
              qDebug () << "Could not convert to QAction" << ja;
            }
          }
        }
#endif
        contextMenu.exec(ui_->jointTree->mapToGlobal(pos));
        return;
      }
    }

    JointTreeItem* JointTreeWidget::buildJointTreeItem(const char* name, ULong& rkConfig, ULong& rkVel)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      HppWidgetsPlugin::JointElement& je = plugin_->jointMap() [name];
      JointTreeItem::NodesPtr_t nodes(je.bodyNames.size());
      for (std::size_t i = 0; i < je.bodyNames.size(); ++i) {
        nodes[i] = main->osg ()->getNode(je.bodyNames[i]);
        // TODO I do not remember why this is important...
        if (!nodes[i]) nodes[i] = main->osg ()->getGroup(je.bodyNames[i]);
      }
      CORBA::Long nbCfg = plugin_->client()->robot ()->getJointConfigSize (name);
      CORBA::Long nbDof = plugin_->client()->robot ()->getJointNumberDof  (name);
      JointTreeItem* j;
      if (nbDof > 0) {
        hpp::floatSeq_var c = plugin_->client()->robot ()->getJointConfig (name);
        hpp::floatSeq_var b = plugin_->client()->robot ()->getJointBounds (name);
        j = new JointTreeItem (name, rkConfig, rkVel, c.in(), b.in(), nbDof, nodes);
      } else {
        j = new JointTreeItem (name, rkConfig, rkVel, hpp::floatSeq(), hpp::floatSeq(), nbDof, nodes);
      }
      je.item = j;
      j->setupActions(plugin_);
      rkConfig += nbCfg;
      rkVel    += nbDof;
      return j;
    }

    void JointTreeWidget::selectJoint(const std::string &jointName)
    {
      HppWidgetsPlugin::JointMap::const_iterator itj = plugin_->jointMap().find(jointName);
      if (itj == plugin_->jointMap().constEnd()) return;
      const HppWidgetsPlugin::JointElement& je = itj.value();
      if (!je.item) return;
      qDebug () << "Selected joint: " << QString::fromStdString(je.name);
      ui_->jointTree->clearSelection();
      ui_->jointTree->setCurrentIndex(je.item->index());
    }

    void JointTreeWidget::openJointBoundDialog(const std::string jointName)
    {
      try {
        hpp::floatSeq_var bounds =
          plugin_->client()->robot()->getJointBounds(jointName.c_str());
        int nbCfg = plugin_->client()->robot()->getJointConfigSize(jointName.c_str());
        if (nbCfg > 0) {
          JointBoundDialog dialog (QString::fromStdString(jointName), nbCfg);
          dialog.setBounds(bounds.in());
          if (dialog.exec() == QDialog::Accepted) {
            dialog.getBounds(bounds.inout());
            plugin_->client()->robot()->setJointBounds(jointName.c_str(), bounds.in());
          }
        }
      } catch (const hpp::Error& e) {
        gepetto::gui::MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
        return;
      }
    }

    void JointTreeWidget::moveJoint(hpp::Transform__slice* transform, std::string const& jointName)
    {
      plugin_->client()->robot()->setJointPositionInParentFrame(jointName.c_str(), transform);
      gepetto::gui::MainWindow::instance()->requestApplyCurrentConfiguration();
    }

    void JointTreeWidget::openJointMoveDialog(const std::string jointName)
    {
      try {
        TransformWidget* d =
	  new TransformWidget(plugin_->client()->robot()->\
			      getJointPositionInParentFrame(jointName.c_str()),
			      jointName, this);

	d->show();
	connect(d, SIGNAL(valueChanged(hpp::Transform__slice*, std::string const&)),
		SLOT(moveJoint(hpp::Transform__slice*, std::string const&)));
      } catch (const hpp::Error& e) {
        gepetto::gui::MainWindow::instance()->logError(QString::fromLocal8Bit(e.msg));
        return;
      }
    }

    void JointTreeWidget::reload()
    {
      reset ();
      plugin_->jointMap ().clear();
      try {
        CORBA::String_var robotName = plugin_->client ()->robot()->getRobotName();
        plugin_->updateRobotJoints(robotName.in());
        hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames ();
        typedef std::map<std::string, JointTreeItem*> JointTreeItemMap_t;
        JointTreeItemMap_t items;
        ULong rkCfg = 0, rkVel = 0;
        for (size_t i = 0; i < joints->length (); ++i) {
          const char* jname = joints[(ULong) i];
          std::string bjn (joints[0]);
          items[jname] = buildJointTreeItem(jname, rkCfg, rkVel);
        }
        for (JointTreeItemMap_t::const_iterator _jti = items.begin();
            _jti != items.end(); ++_jti) {
          CORBA::String_var _pname = plugin_->client()->robot ()->getParentJointName(_jti->first.c_str());
          std::string pname (_pname);
          JointTreeItemMap_t::iterator parent = items.find(pname);
          if (parent == items.end()) model_->        appendRow(_jti->second);
          else                       parent->second->appendRow(_jti->second);
        }
      } catch (const hpp::Error& e) {
        qDebug () << "Could not reload JointTreeWidget:" << e.msg;
        return;
      }
    }

    void JointTreeWidget::reset()
    {
      model_->clear();
      ui_->jointTree->header()->setVisible(true);
      QStringList l; l << "Joint" << "Lower bound" << "Upper bound";
      model_->setHorizontalHeaderLabels(l);
      model_->setColumnCount(3);
      ui_->jointTree->setColumnHidden(1,true);
      ui_->jointTree->setColumnHidden(2,true);
    }

    void JointTreeWidget::resize(const QModelIndex index)
    {
      Q_UNUSED (index);
      ui_->jointTree->resizeColumnToContents(0);
    }

    void JointTreeWidget::currentJointChanged(const QModelIndex& current, const QModelIndex& previous)
    {
      Q_UNUSED (previous);
      if (current.isValid())
      {
        JointTreeItem* item = dynamic_cast <JointTreeItem*>
          (model_->itemFromIndex(current));
        if (item == NULL) return;

        HppWidgetsPlugin::JointMap::const_iterator itj = plugin_->jointMap().find(item->name());
        if (itj == plugin_->jointMap().constEnd()) return;
        const HppWidgetsPlugin::JointElement& je = itj.value();

        MainWindow* main = MainWindow::instance();
        for (std::size_t i = 0; i < je.bodyNames.size(); ++i)
        {
          main->bodyTree()->selectBodyByName (je.bodyNames[i]);
        }
      }
    }
  } // namespace gui
} // namespace hpp
