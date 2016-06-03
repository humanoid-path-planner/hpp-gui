#include "linkwidget.hh"
#include "hppmanipulationwidgetsplugin/ui_linkwidget.h"

namespace hpp {
  namespace gui {
    LinkWidget::LinkWidget(HppManipulationWidgetsPlugin* plugins,
                           QWidget *parent) :
      QWidget(parent),
      ui_(new Ui::LinkWidget)
    {
      ui_->setupUi(this);
      hpp::Names_t_var n;
      QStringList l;
      n = plugins->manipClient()->problem()->getAvailable("gripper");
      for (unsigned i = 0; i < n->length(); ++i) {
        l << n[i].in();
      }
      ui_->grippersList->addItems(l);
      l.clear();
      n = plugins->manipClient()->problem()->getAvailable("handle");
      for (unsigned i = 0; i < n->length(); ++i) {
        l << n[i].in();
      }
      ui_->handlesList->addItems(l);

      connect(ui_->createButton, SIGNAL(clicked()), SLOT(createRule()));

      ui_->rulesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
