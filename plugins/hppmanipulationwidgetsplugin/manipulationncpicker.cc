#include <hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh>
#include "hppwidgetsplugin/ui_numericalconstraintpicker.h"
#include "manipulationncpicker.hh"

namespace hpp {
  namespace gui {
    ManipulationNCPicker::ManipulationNCPicker(QStringList const& names,
                                               HppManipulationWidgetsPlugin* plugin,
                                               QWidget* parent)
        : NumericalConstraintPicker(names, plugin, parent)
    {
      listComp_ = new QListWidget(this);
      hpp::GraphComp_var graphComp;
      hpp::GraphElements_var graphElems;

      plugin->manipClient()->graph()->getGraph(graphComp.out(), graphElems.out());
      components_[graphComp->name.in()] = graphComp;
      listComp_->addItem(graphComp->name.in());
      for (unsigned i = 0; i < graphElems->nodes.length(); ++i) {
        components_[graphElems->nodes[i].name.in()] = graphElems->nodes[i];
        listComp_->addItem(graphElems->nodes[i].name.in());
      }
      for (unsigned i = 0; i < graphElems->edges.length(); ++i) {
        components_[graphElems->edges[i].name.in()] = graphElems->edges[i];
        listComp_->addItem(graphElems->edges[i].name.in());
      }
      listComp_->setSelectionMode(QAbstractItemView::ExtendedSelection);
      QBoxLayout* l = dynamic_cast<QBoxLayout *>(layout());

      l->insertWidget(2, new QLabel("Select on which graph components you want to apply constraints.\n"
                                     "The constraints will be automatically applied to all the components under the one you choose."));
      l->insertWidget(3, listComp_);
    }

    ManipulationNCPicker::~ManipulationNCPicker()
    {
    }

    void ManipulationNCPicker::onConfirmClicked()
    {
      QList<QListWidgetItem*> selectedConst = ui->constraintList->selectedItems();
      hpp::Names_t_var constraints = new hpp::Names_t;
      hpp::Names_t_var dofs = new hpp::Names_t;
      QList<QListWidgetItem*> selectedComp = listComp_->selectedItems();
      HppManipulationWidgetsPlugin* plugin = dynamic_cast<HppManipulationWidgetsPlugin*>(plugin_);

      constraints->length(selectedConst.count());
      int i = 0;
      foreach (QListWidgetItem* item, selectedConst) {
        constraints[i] = item->text().toStdString().c_str();
      }
      foreach (QListWidgetItem* item, selectedComp) {
        plugin->manipClient()->graph()->setNumericalConstraints(components_[item->text().toStdString()].id,
            constraints.in(), dofs.in());
      }
      onCancelClicked();
    }
  }
}
