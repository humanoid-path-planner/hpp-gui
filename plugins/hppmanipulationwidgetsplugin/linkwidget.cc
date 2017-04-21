#include "linkwidget.hh"
#include "hppmanipulationwidgetsplugin/ui_linkwidget.h"

namespace hpp {
  namespace gui {
    LinkWidget::LinkWidget(QListWidget* grippersList, QListWidget* handlesList,
                           QWidget *parent) :
      QWidget(parent),
      ui_(new Ui::LinkWidget)
    {
      ui_->setupUi(this);
      grippers_ = grippersList;
      handles_ = handlesList;

      connect(ui_->createButton, SIGNAL(clicked()), SLOT(createRule()));

      QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui_->rulesList);
      connect(shortcut, SIGNAL(activated()), this, SLOT(deleteSelectedRules()));

      ui_->rulesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

      connect(grippersList->selectionModel(),
	      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	      SLOT(gripperChanged(const QItemSelection&, const QItemSelection&)));
      connect(handlesList->selectionModel(),
	      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	      SLOT(handleChanged(const QItemSelection&, const QItemSelection&)));

      ui_->handlesList->addItem(".*");
      ui_->handlesList->addItem("");
    }

    LinkWidget::~LinkWidget()
    {
      delete ui_;
    }

    Rules_var LinkWidget::getRules()
    {
      Rules_var rules = new Rules();
      rules->length(rules_.size());

      int i = 0;
      foreach (const RuleProxy& rule, rules_) {
        rules[i].grippers.length(rule.grippers.size());
        rules[i].handles .length(rule.handles .size());
        for(std::size_t j = 0; j < rule.grippers.size(); ++j) {
          QByteArray a = rule.grippers[j].toLocal8Bit();
          rules[i].grippers[j] = new char[a.length()+1];
          strcpy(rules[i].grippers[0], a.constData());
        }
        for(std::size_t j = 0; j < rule.grippers.size(); ++j) {
          QByteArray a = rule.handles[j].toLocal8Bit();
          rules[i].handles[j] = new char[a.length()+1];
          strcpy(rules[i].handles[j], a.constData());
        }
        rules[i].link = rule.link;
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
          rules_.push_back(RuleProxy());
          RuleProxy& rule = rules_.back();

          QString gripperName = gripper->text();
          QString handleName  = handle->text();

          rule.grippers.push_back(gripperName);
          rule.handles.push_back(handleName);

          rule.link = ui_->linked->isChecked();

          QString text (((rule.link == true) ? "Link " : "Don't link ")
                       + gripperName + " and " + handleName);
          ui_->rulesList->addItem(text);
        }
      }
    }

    void LinkWidget::deleteSelectedRules()
    {
      foreach (QListWidgetItem *item, ui_->rulesList->selectedItems()) {
        for (std::size_t row = 0; row < ui_->rulesList->count(); ++row)
          if (item == ui_->rulesList->item(row)) {
            rules_.remove(row);
            ui_->rulesList->takeItem(row);
            break;
          }
      }
    }
  }
}
