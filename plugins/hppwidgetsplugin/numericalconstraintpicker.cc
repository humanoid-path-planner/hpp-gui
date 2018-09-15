//
// Copyright (c) CNRS
// Authors: Heidy Dallard
//

#include "hppwidgetsplugin/numericalconstraintpicker.hh"

#include <iostream>
#include <QHBoxLayout>

#include "hppwidgetsplugin/ui_numericalconstraintpicker.h"
#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    NumericalConstraintPicker::NumericalConstraintPicker(HppWidgetsPlugin* plugin,
                                                         QWidget *parent) :
      QWidget(parent),
      ui(new Ui::NumericalConstraintPicker)
    {
      ui->setupUi(this);

      hpp::Names_t_var names = plugin->client()->problem()->getAvailable("lockedjoint");
      ui->lockedJointList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      for (unsigned i = 0; i < names->length(); ++i) {
          ui->lockedJointList->addItem(QString(names[i]));
      }

      names = plugin->client()->problem()->getAvailable("numericalconstraint");
      ui->numericalList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      for (unsigned i = 0; i < names->length(); ++i) {
        ui->numericalList->addItem(QString(names[i]));
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
      QList<QListWidgetItem *> lj = ui->lockedJointList->selectedItems();
      QList<QListWidgetItem *> nc = ui->numericalList->selectedItems();

      hpp::Names_t_var names = new hpp::Names_t;
      hpp::intSeq_var priorities = new hpp::intSeq;

      names->length(nc.count());
      priorities->length(nc.count());
      int i = 0;
      foreach(QListWidgetItem* item, nc) {
        names[i] = gepetto::gui::Traits<QString>::to_corba(item->text());
        priorities[i] = 0;
        i++;
      }
      plugin_->client()->problem()->addNumericalConstraints(gepetto::gui::Traits<QString>::to_corba(ui->constraintName->text()),
                                                            names.in(), priorities.in());

      names->length(lj.count());
      i = 0;
      foreach(QListWidgetItem* item, lj) {
        names[i] = gepetto::gui::Traits<QString>::to_corba(item->text());
        i++;
      }
      plugin_->client()->problem()->addLockedJointConstraints(gepetto::gui::Traits<QString>::to_corba(ui->constraintName->text()),
                                                              names.in());
      onCancelClicked();
    }
  }
}
