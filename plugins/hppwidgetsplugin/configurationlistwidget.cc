#include "hppwidgetsplugin/configurationlistwidget.hh"
#include "hppwidgetsplugin/ui_configurationlistwidget.h"

#include "hpp/corbaserver/common.hh"
#include "hpp/corbaserver/client.hh"

namespace hpp {
  namespace gui {
    const int ConfigurationListWidget::ConfigRole = Qt::UserRole + 1;

    ConfigurationListWidget::ConfigurationListWidget(HppWidgetsPlugin *plugin, QWidget* parent) :
      QWidget (parent),
      plugin_ (plugin),
      ui_ (new ::Ui::ConfigurationListWidget),
      main_ (gepetto::gui::MainWindow::instance()),
      basename_ ("config_"), count_ (0)
    {
      ui_->setupUi (this);

      connect (ui_->button_SaveConfig, SIGNAL (clicked()), this, SLOT (onSaveClicked()));
      connect (ui_->button_ResetGoalConfig, SIGNAL (clicked()), SLOT(resetGoalConfigs()));
      connect (list (), SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect (list (), SIGNAL (customContextMenuRequested(QPoint)),
          this, SLOT (showListContextMenu (QPoint)));
    }

    ConfigurationListWidget::~ConfigurationListWidget()
    {
      delete ui_;
    }

    QListWidget *ConfigurationListWidget::list() {
      return ui_->listConfigurations;
    }

    void ConfigurationListWidget::onSaveClicked ()
    {
      hpp::floatSeq* c = plugin_->client()->robot()->getCurrentConfig ();
      QVariant var;
      var.setValue (c);
      QLineEdit* n = name();
      QListWidgetItem* newItem = new QListWidgetItem (n->text());
      newItem->setData(ConfigRole, var);
      list()->addItem(newItem);
      n->setText(basename_ + QString::number(count_));
      count_++;
    }

    void ConfigurationListWidget::updateCurrentConfig (QListWidgetItem* current, QListWidgetItem *)
    {
      if (current != 0) {
          const hpp::floatSeq& c = *(current->data(ConfigRole).value <hpp::floatSeq*> ());
          plugin_->client()->robot()->setCurrentConfig (c);
          main_->requestApplyCurrentConfiguration();
        }
    }

    void ConfigurationListWidget::renameConfig(QListWidgetItem* item)
    {
      QDialog* d = new QDialog;
      QVBoxLayout* layout = new QVBoxLayout(d);
      QLineEdit* newName = new QLineEdit(d);
      QPushButton* button = new QPushButton("Confirm", d);

      layout->addWidget(new QLabel("Name :", d));
      layout->addWidget(newName);
      layout->addWidget(button);
      connect(button, SIGNAL(clicked()), d, SLOT(close()));
      d->setLayout(layout);
      d->exec();
      if (newName->text() != "") {
        item->setText(newName->text());
      }
    }

    void ConfigurationListWidget::showListContextMenu (const QPoint& pos)
    {
      QListWidgetItem* item = list()->itemAt(pos);
      if (!item) return;
      QMenu contextMenu(tr("Configuration"), this);
      QAction* init = new QAction(tr("Set as initial configuration"), this);
      QAction* goal = new QAction(tr("Add as goal configuration"), this);
      QAction* reset = new QAction(tr("Reset goal configurations"), this);
      QAction* rename = new QAction(tr("Rename configuration"), this);
      contextMenu.addAction(init);
      contextMenu.addAction(goal);
      contextMenu.addSeparator();
      contextMenu.addAction(reset);
      contextMenu.addSeparator();
      contextMenu.addAction(rename);
      QAction* selected = contextMenu.exec(mapToGlobal(pos));
      const hpp::floatSeq& c = *(item->data(ConfigRole).value <hpp::floatSeq*> ());
      if (selected == init) {
        plugin_->client()->problem()->setInitialConfig (c);
      } else if (selected == goal) {
        plugin_->client()->problem()->addGoalConfig (c);
      } else if (selected == reset) {
        plugin_->client()->problem()->resetGoalConfigs ();
        }
      else if (selected == rename) {
        renameConfig(item);
      }
    }

    void ConfigurationListWidget::resetGoalConfigs()
    {
      plugin_->client()->problem()->resetGoalConfigs();
    }

    QLineEdit *ConfigurationListWidget::name() {
      return ui_->lineEdit_configName;
    }
  } // namespace gui
} // namespace hpp
