//
// Copyright (c) CNRS
// Authors: Heidy Dallard
//

#include <QHBoxLayout>
#include "hppwidgetsplugin/numericalconstraintpicker.hh"
#include "hppwidgetsplugin/ui_numericalconstraintpicker.h"

#include <iostream>

namespace hpp {
  namespace gui {
    NumericalConstraintPicker::NumericalConstraintPicker(QStringList const& names,
                                                         HppWidgetsPlugin* plugin,
                                                         QWidget *parent) :
      QWidget(parent),
      ui(new Ui::NumericalConstraintPicker)
    {
      ui->setupUi(this);
      ui->constraintList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      foreach (QString name, names) {
        ui->constraintList->addItem(name);
      }
      connect(ui->cancelButton, SIGNAL(clicked()), SLOT(onCancelClicked()));
      connect(ui->confirmButton, SIGNAL(clicked()), SLOT(onConfirmClicked()));
      plugin_ = plugin;
    }

    NumericalConstraintPicker::~NumericalConstraintPicker()
    {
      delete ui;
    }

    void NumericalConstraintPicker::closeEvent(QCloseEvent* event)
    {
      deleteLater();
      event->accept();
    }

    void NumericalConstraintPicker::onCancelClicked()
    {
      deleteLater();
      close();
    }

    void NumericalConstraintPicker::onConfirmClicked()
    {
      QList<QListWidgetItem *> list = ui->constraintList->selectedItems();
      hpp::Names_t_var names = new hpp::Names_t;
      hpp::intSeq_var priorities = new hpp::intSeq;

      names->length(list.count());
      priorities->length(list.count());
      int i = 0;
      foreach(QListWidgetItem* item, list) {
        names[i] = gepetto::gui::Traits<QString>::to_corba(item->text());
        priorities[i] = 0;
        i++;
      }
      plugin_->client()->problem()->setNumericalConstraints(gepetto::gui::Traits<QString>::to_corba(ui->numericalName->text()),
                                                            names.in(), priorities.in());
      onCancelClicked();
    }
  }
}
