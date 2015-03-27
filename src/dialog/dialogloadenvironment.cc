#include "hpp/gui/dialog/dialogloadenvironment.h"
#include "ui_dialogloadenvironment.h"

#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

QList <DialogLoadEnvironment::EnvironmentDefinition> DialogLoadEnvironment::definitions =
    QList <DialogLoadEnvironment::EnvironmentDefinition>()
    << DialogLoadEnvironment::EnvironmentDefinition ()
    << DialogLoadEnvironment::EnvironmentDefinition ("Kitchen", "kitchen", "iai_maps",
                                         "/local/jmirabel/devel/release/install/share/iai_maps/",
                                         "kitchen_area", "/local/jmirabel/devel/release/install/share/iai_maps/")
    << DialogLoadEnvironment::EnvironmentDefinition ("Floor with bar", "floor_with_bar", "iai_maps",
                                         "/local/jmirabel/devel/release/install/share/iai_maps/",
                                         "floor_with_bar", "/local/jmirabel/devel/release/install/share/iai_maps/");

DialogLoadEnvironment::DialogLoadEnvironment(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::DialogLoadEnvironment)
{
  ui_->setupUi(this);
  ui_->groupBox_details->setVisible (false);
  defs_  = ui_->comboBox_defs;

  foreach (EnvironmentDefinition r, definitions) {
      defs_->addItem(r.name_, QVariant::fromValue(r));
    }
}

DialogLoadEnvironment::~DialogLoadEnvironment()
{
  delete ui_;
}

void DialogLoadEnvironment::addEnvironmentDefinition(QString name, QString envName, QString package, QString packagePath, QString urdfFilename, QString meshDirectory)
{
  definitions.append(EnvironmentDefinition (name, envName, package, packagePath, urdfFilename, meshDirectory));
}

void DialogLoadEnvironment::accept()
{
  QDir d (ui_->packagePath->text ());
  if (!d.cd("urdf")) {
      QMessageBox (QMessageBox::Warning, "Directory not found", d.absolutePath(), QMessageBox::Ok, this).exec();
      return;
    }
  if (!d.exists(ui_->urdfFilename->text () + ".urdf")) {
      QMessageBox (QMessageBox::Warning, "File not found", d.absoluteFilePath(ui_->urdfFilename->text() + ".urdf"), QMessageBox::Ok, this).exec();
      return;
    }
  if (!QDir (ui_->meshDirectory->text ()).exists()) {
      QMessageBox (QMessageBox::Warning, "File not found", ui_->meshDirectory->text (), QMessageBox::Ok, this).exec();
      return;
    }
  selected_ = EnvironmentDefinition (defs_->currentText() ,
                                     ui_->envName->text(),
                                     ui_->packageName->text(),
                                     ui_->packagePath->text(),
                                     ui_->urdfFilename->text(),
                                     ui_->meshDirectory->text());
  done(QDialog::Accepted);
}

void DialogLoadEnvironment::meshSelect()
{
  ui_->meshDirectory->setText(
        QFileDialog::getExistingDirectory(
          this, "Mesh data directory", ui_->meshDirectory->text()));
}

void DialogLoadEnvironment::packagePathSelect()
{
  QDir path (QFileDialog::getExistingDirectory(this, "Package", ui_->packagePath->text ()));
  ui_->packagePath->setText(path.absolutePath());
  ui_->packageName->setText(path.dirName());
  if (ui_->meshDirectory->text().isEmpty())
    ui_->meshDirectory->setText(path.absolutePath());
}

void DialogLoadEnvironment::envSelect(int index)
{
  QVariant v = defs_->itemData(index);
  if (v.canConvert <EnvironmentDefinition>()) {
      EnvironmentDefinition rd = v.value <EnvironmentDefinition> ();
      ui_->envName->setText(rd.envName_);
      ui_->packageName->setText(rd.package_);
      ui_->packagePath->setText(rd.packagePath_);
      ui_->urdfFilename->setText(rd.urdfFilename_);
      ui_->meshDirectory->setText(rd.mesh_);
    }
}
