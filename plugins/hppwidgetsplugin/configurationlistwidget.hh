#ifndef HPP_GUI_CONFIGURATIONLISTWIDGET_HH
#define HPP_GUI_CONFIGURATIONLISTWIDGET_HH

#include <QWidget>

#include "hpp/gui/fwd.hh"
#include "hpp/gui/mainwindow.hh"

#include "hpp/corbaserver/common.hh"

#include <hppwidgetsplugin/hppwidgetsplugin.hh>

Q_DECLARE_METATYPE (hpp::floatSeq*)

namespace Ui {
  class ConfigurationListWidget;
}

class ConfigurationListWidget : public QWidget
{
  Q_OBJECT

public:
  static const int ConfigRole;

  ConfigurationListWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);

  virtual ~ConfigurationListWidget();

public slots:
  void onSaveClicked ();
  void updateCurrentConfig (QListWidgetItem* item);
  void showListContextMenu (const QPoint& pos);

private:
  inline QListWidget* list ();
  inline QLineEdit* name ();

  HppWidgetsPlugin* plugin_;
  Ui::ConfigurationListWidget* ui_;

  MainWindow* main_;
  QString basename_;
  int count_;
};

#endif // HPP_GUI_CONFIGURATIONLISTWIDGET_HH
