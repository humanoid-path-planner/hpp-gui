#include <omniORB4/CORBA.h>
#include "hpp/gui/meta.hh"
#include "hpp/gui/mainwindow.hh"

#include "hppwidgetsplugin/constraintwidget.hh"
#include "hppwidgetsplugin/transformconstraintwidget.hh"
#include "hppwidgetsplugin/numericalconstraintpicker.hh"
#include "hppwidgetsplugin/ui_constraintwidget.h"

namespace hpp {
  namespace gui {
    ConstraintWidget::ConstraintWidget(HppWidgetsPlugin* plugin, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ConstraintWidget)
    {
        ui->setupUi(this);
        plugin_ = plugin;

        connect(ui->firstJointSelect, SIGNAL(currentIndexChanged(int)),
                SLOT(firstJointSelect(int)));
        connect(ui->createConstraintButton, SIGNAL(clicked()), SLOT(createConstraint()));
        connect(ui->resetButton, SIGNAL(clicked()), SLOT(reset()));
        connect(ui->confirmButton, SIGNAL(clicked()), SLOT(confirmNumerical()));
        connect(ui->applyButton, SIGNAL(clicked()), SLOT(applyConstraints()));
        connect(ui->firstGlobalFrame, SIGNAL(toggled(bool)), SLOT(globalSelected(bool)));
    }

    ConstraintWidget::~ConstraintWidget()
    {
      funcs_.clear();
      delete ui;
    }

    void ConstraintWidget::addConstraint(IConstraint *constraint)
    {
      funcs_[(int)funcs_.size()] = constraint;
      ui->constraintTypeSelect->addItem(constraint->getName());
      connect(constraint, SIGNAL(finished(QString)), SLOT(createFinished(QString)));
    }

    void ConstraintWidget::reset()
    {
      plugin_->client()->problem()->resetConstraints();
      //ui->nameList->clear();
    }

    void ConstraintWidget::applyConstraints()
    {
      hpp::floatSeq_var config = plugin_->client()->robot()->getCurrentConfig();
      hpp::floatSeq_var newConfig = NULL;
      double residual;

      if (plugin_->client()->problem()->applyConstraints(config.in(), newConfig.out(), residual) == false) {
        MainWindow::instance()->logError("Could not apply constraints to current configuration");
      }
      else {
        plugin_->client()->robot()->setCurrentConfig(newConfig.in());
        MainWindow::instance()->requestApplyCurrentConfiguration();
      }
    }

    void ConstraintWidget::confirmNumerical()
    {
      QStringList l;
      for (int i = 0; i < ui->nameList->count(); i++) {
        QListWidgetItem* item = ui->nameList->item(i);
        l << item->text();
      }
      NumericalConstraintPicker* ncp = new NumericalConstraintPicker(l, plugin_);

      ncp->show();
    }

    void ConstraintWidget::reload()
    {
      try {
        joints_ = plugin_->client()->robot()->getAllJointNames();
        ui->firstJointSelect->clear();
        for (unsigned i = 0; i < joints_->length(); i++) {
          ui->firstJointSelect->addItem(joints_[i].in());
        }
        firstJointSelect(0);
      }
      catch (hpp::Error& e) {
        MainWindow::instance ()->logError(QString(e.msg));
      }
    }

    void ConstraintWidget::createConstraint()
    {
      QString name = ui->constraintNameEdit->text();
      QString firstJoint = (ui->firstGlobalFrame->isChecked()) ? "" : ui->firstJointSelect->currentText();
      QString secondJoint = (ui->secondGlobalFrame->isChecked()) ? "" : ui->secondJointSelect->currentText();

      if (name == "" || (firstJoint == "" && secondJoint == "")) {
        QMessageBox::information(this, "hpp-gui", "You have to give a name and select at least one joint");
        return ;
      }
      if (funcs_.find(ui->constraintTypeSelect->currentIndex()) != funcs_.end()) {
        (*(this->funcs_[ui->constraintTypeSelect->currentIndex()]))(name, firstJoint, secondJoint);
      }
    }

    void ConstraintWidget::firstJointSelect(int index)
    {
      ui->secondJointSelect->clear();
      for (unsigned i = 0; i < joints_->length(); i++) {
        if (i != index) ui->secondJointSelect->addItem(joints_[i].in());
      }
    }

    void ConstraintWidget::globalSelected(bool action)
    {
      if (action) {
        firstJointSelect(-1);
      }
      else {
        firstJointSelect(ui->firstJointSelect->currentIndex());
      }
    }

    void ConstraintWidget::createFinished(QString name)
    {
      ui->nameList->addItem(name);
      ui->constraintNameEdit->clear();
    }
  }
}
