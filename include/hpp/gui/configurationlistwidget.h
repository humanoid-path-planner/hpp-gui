#ifndef CONFIGURATIONLISTWIDGET_H
#define CONFIGURATIONLISTWIDGET_H

#include <QWidget>

#include "hpp/gui/fwd.h"
#include "hpp/gui/mainwindow.h"

#include "hpp/corbaserver/common.hh"

Q_DECLARE_METATYPE (hpp::floatSeq*)

class ConfigurationListWidget : public QWidget
{
  Q_OBJECT

public:
  static const int ConfigRole;

  ConfigurationListWidget(QWidget* parent = 0);

  virtual ~ConfigurationListWidget();

  inline QListWidget* list () {
    return findChild <QListWidget*> ("listConfigurations");
  }

public slots:
  void onSaveClicked ();
  void updateCurrentConfig (QListWidgetItem* item);
  void showListContextMenu (const QPoint& pos);

private:
  inline QLineEdit* name () {
    return findChild <QLineEdit*> ("lineEdit_configName");
  }

  MainWindow* main_;
  QString basename_;
  int count_;
};

#endif // CONFIGURATIONLISTWIDGET_H
