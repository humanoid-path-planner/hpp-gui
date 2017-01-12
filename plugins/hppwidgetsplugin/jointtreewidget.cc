#include <omniORB4/CORBA.h>

#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppwidgetsplugin/ui_jointtreewidget.h"

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>

#include "hppwidgetsplugin/joint-tree-item.hh"
#include "hppwidgetsplugin/jointbounddialog.hh"
#include "hppwidgetsplugin/transformwidget.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
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

      connect(ui_->jointTree, SIGNAL (customContextMenuRequested(QPoint)),
          SLOT (customContextMenu(QPoint)));
      connect(ui_->jointTree, SIGNAL (expanded(QModelIndex)),
          SLOT (resize(QModelIndex)));
    }

    JointTreeWidget::~JointTreeWidget()
    {
      delete ui_;
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
        contextMenu.addActions (plugin_->getJointActions(item->name()));
        foreach (gepetto::gui::JointModifierInterface* adi,
            gepetto::gui::MainWindow::instance()->pluginManager ()->get<gepetto::gui::JointModifierInterface> ()) {
          if (!adi) continue;
          contextMenu.addAction (adi->action (item->name()));
        }
        contextMenu.exec(ui_->jointTree->mapToGlobal(pos));
        return;
      }
    }

    void JointTreeWidget::addJointToTree(const std::string name, JointTreeItem *parent)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      HppWidgetsPlugin::JointElement& je = plugin_->jointMap() [name];
      JointTreeItem::NodesPtr_t nodes(je.bodyNames.size());
      for (std::size_t i = 0; i < je.bodyNames.size(); ++i) {
        nodes[i] = main->osg ()->getNode(je.bodyNames[i]);
        // TODO I do not remember why this is important...
        if (!nodes[i]) nodes[i] = main->osg ()->getGroup(je.bodyNames[i]);
      }
      hpp::floatSeq_var c = plugin_->client()->robot ()->getJointConfig (name.c_str());
      CORBA::Short nbDof = plugin_->client()->robot ()->getJointNumberDof (name.c_str());
      hpp::corbaserver::jointBoundSeq_var b = plugin_->client()->robot ()->getJointBounds (name.c_str());

      JointTreeItem* j = new JointTreeItem (name.c_str(), c.in(), b.in(), nbDof, nodes);
      je.item = j;
      if (parent) parent->appendRow(j);
      else        model_->appendRow(j);
      hpp::Names_t_var children = plugin_->client()->robot ()->getChildJointNames (name.c_str());
      for (size_t i = 0; i < children->length(); ++i)
        addJointToTree(std::string(children[(ULong) i]),j);
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
        hpp::corbaserver::jointBoundSeq_var bounds =
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
      char* robotName;
      try {
        robotName = plugin_->client ()->robot()->getRobotName();
        hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames ();
        std::string bjn (joints[0]);
        plugin_->updateRobotJoints(robotName);
        addJointToTree(bjn, 0);
      } catch (hpp::Error& e) {
        gepetto::gui::MainWindow::instance ()->logError(QString(e.msg));
        return;
      }
      delete[] robotName;
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
  } // namespace gui
} // namespace hpp
