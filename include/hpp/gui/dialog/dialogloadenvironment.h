#ifndef DIALOGLOADENVIRONMENT_H
#define DIALOGLOADENVIRONMENT_H

#include <QDialog>
#include <QComboBox>

namespace Ui {
  class DialogLoadEnvironment;
}

class DialogLoadEnvironment : public QDialog
{
  Q_OBJECT
  
public:
  explicit DialogLoadEnvironment(QWidget *parent = 0);
  ~DialogLoadEnvironment();

  struct EnvironmentDefinition {
    QString name_, envName_, urdfFilename_, mesh_, package_, packagePath_;
    EnvironmentDefinition () {}
    EnvironmentDefinition (QString name, QString envName,
                           QString package, QString packagePath,
                           QString urdfFilename, QString meshDirectory) :
      name_(name), envName_ (envName), urdfFilename_(urdfFilename),
      mesh_(meshDirectory), package_ (package), packagePath_ (packagePath)
    {}
  };

  static void addEnvironmentDefinition (QString name,
                                        QString envName,
                                        QString package,
                                        QString packagePath,
                                        QString urdfFilename,
                                        QString meshDirectory);
  static QList <EnvironmentDefinition> getEnvironmentDefinitions ();

  EnvironmentDefinition getSelectedDescription () {
    return selected_;
  }

private slots:
  void accept();
  void meshSelect();
  void packagePathSelect();
  void envSelect(int index);

private:
  Ui::DialogLoadEnvironment *ui_;
  QComboBox* defs_;
  EnvironmentDefinition selected_;

  static QList <EnvironmentDefinition> definitions;
};

Q_DECLARE_METATYPE (DialogLoadEnvironment::EnvironmentDefinition)

#endif // DIALOGLOADENVIRONMENT_H
