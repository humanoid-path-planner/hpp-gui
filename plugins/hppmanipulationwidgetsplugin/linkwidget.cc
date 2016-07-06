#include "linkwidget.hh"
#include "hppmanipulationwidgetsplugin/ui_linkwidget.h"

namespace hpp {
  namespace gui {
    LinkWidget::LinkWidget(HppManipulationWidgetsPlugin* plugins,
			   QListWidget* grippersList, QListWidget* handlesList,
                           QWidget *parent) :
      QWidget(parent),
      ui_(new Ui::LinkWidget)
    {
      ui_->setupUi(this);
      grippers_ = grippersList;
      handles_ = handlesList;

      connect(ui_->createButton, SIGNAL(clicked()), SLOT(createRule()));

      ui_->rulesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

      connect(grippersList->selectionModel(),
	      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	      SLOT(gripperChanged(const QItemSelection&, const QItemSelection&)));
      connect(handlesList->selectionModel(),
	      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	      SLOT(handleChanged(const QItemSelection&, const QItemSelection&)));
    }

    LinkWidget::~LinkWidget()
    {
      delete ui_;
    }

    Rules_var LinkWidget::getRules()
    {
      QList<QListWidgetItem *> selected = ui_->rulesList->selectedItems();
      Rules_var rules = new Rules();

      int i = 0;
      rules->length(selected.count());
      foreach (QListWidgetItem *item, selected) {
        int row = ui_->rulesList->row(item);

        rules[i] = rules_[row];
        ++i;
      }
      return rules;
    }

    void LinkWidget::gripperChanged(const QItemSelection& selected,
				    const QItemSelection& deselected)
    {
      foreach(QModelIndex idx, selected.indexes()) {
	ui_->grippersList->addItem(grippers_->item(idx.row())->text());
      }
      foreach(QModelIndex idx, deselected.indexes()) {
	QList<QListWidgetItem*> list = ui_->grippersList->findItems(grippers_->item(idx.row())->text(), Qt::MatchExactly);
        delete ui_->grippersList->takeItem(ui_->grippersList->row(list.front()));
      }
    }

    void LinkWidget::handleChanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
      foreach(QModelIndex idx, selected.indexes()) {
	ui_->handlesList->addItem(handles_->item(idx.row())->text());
      }
      foreach(QModelIndex idx, deselected.indexes()) {
	QList<QListWidgetItem*> list = ui_->handlesList->findItems(handles_->item(idx.row())->text(), Qt::MatchExactly);
        delete ui_->handlesList->takeItem(ui_->handlesList->row(list.front()));
      }
    }

    void LinkWidget::createRule()
    {
      QList<QListWidgetItem *> grippers = ui_->grippersList->selectedItems();
      QList<QListWidgetItem *> handles = ui_->handlesList->selectedItems();

      foreach (QListWidgetItem *gripper, grippers) {
        foreach (QListWidgetItem *handle, handles) {
          hpp::corbaserver::manipulation::Rule rule;
          std::string gripperName = gripper->text().toStdString();
          std::string handleName = handle->text().toStdString();

          rule.gripper = gripperName.c_str();
          rule.handle = handleName.c_str();
          rule.link = ui_->linked->isChecked();
          rules_.push_back(rule);
          std::string text(((rule.link == true) ? "Link " : "Don't link ")
                       + gripperName + " and " + handleName);
          ui_->rulesList->addItem(text.c_str());
        }
      }
    }
  }
}
