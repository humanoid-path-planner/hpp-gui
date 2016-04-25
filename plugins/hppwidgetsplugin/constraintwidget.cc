#include <omniORB4/CORBA.h>
#include "hpp/gui/meta.hh"
#include "hpp/gui/mainwindow.hh"

#include "hppwidgetsplugin/constraintwidget.hh"
#include "hppwidgetsplugin/positionconstraintwidget.hh"
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
    }

    ConstraintWidget::~ConstraintWidget()
    {
      funcs_.clear();
      delete ui;
    }

    void ConstraintWidget::addConstraint(IConstraint *constraint)
    {
      funcs_[funcs_.size()] = constraint;
      ui->constraintTypeSelect->addItem(constraint->getName());
      connect(constraint, SIGNAL(finished(QString)), SLOT(createFinished(QString)));
    }

    void ConstraintWidget::reset()
    {
      plugin_->client()->problem()->resetConstraints();
      ui->nameList->clear();
    }

    void ConstraintWidget::applyConstraints()
    {
      hpp::floatSeq_var config = plugin_->client()->robot()->getCurrentConfig();
      hpp::floatSeq_var newConfig = NULL;
      double residual;

      if (plugin_->client()->problem()->applyConstraints(config.in(), newConfig, residual) == false) {
        MainWindow::instance()->logError("Could not apply constraints to current configuration");
      }
      else {
          plugin_->client()->robot()->setCurrentConfig(newConfig.in());
          MainWindow::instance()->requestApplyCurrentConfiguration();
      }
    }

    void ConstraintWidget::confirmNumerical()
    {
      QString name("hpp-gui-constraints");
      QListWidget* list = ui->nameList;
      hpp::Names_t_var constraintsNames = new hpp::Names_t;
      hpp::intSeq_var priorities = new hpp::intSeq;
      int count = list->count();

      constraintsNames->length(count);
      priorities->length(count);
      for (int i = 0; i < count; ++i) {
        constraintsNames[i] = Traits<QString>::to_corba(list->item(i)->text());
        priorities[i] = 0;
      }
      plugin_->client()->problem()->setNumericalConstraints(Traits<QString>::to_corba(name),
                                                            constraintsNames.in(),
                                                            priorities.in());
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
      QString firstJoint = ui->firstJointSelect->currentText();
      QString secondJoint = ui->secondJointSelect->currentText();

      if (name == "" || secondJoint == "") {
        QMessageBox::information(this, "hpp-gui", "You have to fill all the field to create a constraint");
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

    void ConstraintWidget::createFinished(QString name)
    {
      ui->nameList->addItem(name);
      ui->constraintNameEdit->clear();
    }
  }
}
