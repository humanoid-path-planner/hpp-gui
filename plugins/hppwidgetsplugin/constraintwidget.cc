#include <omniORB4/CORBA.h>
#include "gepetto/gui/meta.hh"
#include "gepetto/gui/mainwindow.hh"

#include "hppwidgetsplugin/constraintwidget.hh"
#include "hppwidgetsplugin/transformconstraintwidget.hh"
#include "hppwidgetsplugin/numericalconstraintpicker.hh"
#include "hppwidgetsplugin/ui_constraintwidget.h"

using gepetto::gui::MainWindow;

namespace hpp {
  namespace gui {
    ConstraintWidget::ConstraintWidget(HppWidgetsPlugin* plugin, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ConstraintWidget)
    {
        haveWidget = false;
        ui->setupUi(this);
        plugin_ = plugin;

        connect(ui->createButton, SIGNAL(clicked()), SLOT(createConstraint()));
        connect(ui->resetButton, SIGNAL(clicked()), SLOT(reset()));
        connect(ui->confirmButton, SIGNAL(clicked()), SLOT(confirmNumerical()));
        connect(ui->applyButton, SIGNAL(clicked()), SLOT(applyConstraints()));
        connect(ui->constraintTypeSelect, SIGNAL(currentIndexChanged(int)), SLOT(typeChanged(int)));
	lastInsert_ = 0;
	ui->constraintNameEdit->setText("constraint_0");
        MainWindow* main = MainWindow::instance();
	connect(main, SIGNAL(refresh()), SLOT(refresh()));
    }

    ConstraintWidget::~ConstraintWidget()
    {
      funcs_.clear();
      delete ui;
    }

    void ConstraintWidget::addConstraint(IConstraint *constraint)
    {
      funcs_.push_back(constraint);
      ui->constraintTypeSelect->addItem(constraint->getName());
      connect(constraint, SIGNAL(constraintCreated(QString)), SLOT(onConstraintCreated(QString)));
      connect(constraint, SIGNAL(finished()), SLOT(onFinished()));
    }

    void ConstraintWidget::reset()
    {
      plugin_->client()->problem()->resetConstraints();
    }

    void ConstraintWidget::applyConstraints()
    {
      hpp::floatSeq_var config = plugin_->client()->robot()->getCurrentConfig();
      hpp::floatSeq_var newConfig = NULL;
      double residual;

      if (plugin_->client()->problem()->applyConstraints(config.in(), newConfig.out(), residual) == false) {
        gepetto::gui::MainWindow::instance()->logError("Could not apply constraints to current configuration");
      }
      else {
        plugin_->client()->robot()->setCurrentConfig(newConfig.in());
        gepetto::gui::MainWindow::instance()->requestApplyCurrentConfiguration();
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
      for (std::vector<IConstraint *>::iterator it = funcs_.begin(); it != funcs_.end(); ++it) {
        try {
          (*it)->reload();
        }
        catch (hpp::Error& e) {
          gepetto::gui::MainWindow::instance ()->logError(QString(e.msg));
        }
      }
    }

    void ConstraintWidget::refresh()
    {
      ui->nameList->clear();
      hpp::Names_t_var nc = plugin_->client()->problem()->getAvailable("NumericalConstraint");
      for (unsigned i = 0; i < nc->length(); i++) {
	ui->nameList->addItem(nc[i].in());
      }
    }

    void ConstraintWidget::createConstraint()
    {
      QString name = ui->constraintNameEdit->text();

      if (name == "") {
        QMessageBox::information(this, "hpp-gui", "You have to give a name and select at least one joint");
        return ;
      }
      if (ui->constraintTypeSelect->currentIndex() < funcs_.size()) {
        (*(this->funcs_[ui->constraintTypeSelect->currentIndex()]))(name);
      }
    }

    void ConstraintWidget::onConstraintCreated(QString name)
    {
      ui->nameList->addItem(name);
    }

    void ConstraintWidget::onFinished()
    {
      lastInsert_++;
      ui->constraintNameEdit->setText("constraint_" + QString::number(lastInsert_));
    }

    void ConstraintWidget::typeChanged(int index)
    {
      QBoxLayout* layoutVar = dynamic_cast<QBoxLayout *>(layout());


      if (index < funcs_.size()) {
        if (haveWidget) {
          layout()->takeAt(2)->widget()->hide();
        }
        QWidget* toAdd = funcs_[index]->getWidget();

        toAdd->show();
        layoutVar->insertWidget(2, toAdd);
        haveWidget = true;
      }
    }
  }
}
