//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include "hppwidgetsplugin/configurationlistwidget.hh"
#include "hppwidgetsplugin/ui_configurationlistwidget.h"

#include "hpp/corbaserver/client.hh"

#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace hpp {
  namespace gui {
    const int ConfigurationListWidget::ConfigRole = Qt::UserRole + 1;

    namespace {
      class ConfigItem : public QListWidgetItem
      {
      public:
        ConfigItem (QString text, const hpp::floatSeq& q)
          : QListWidgetItem (text)
          , q_ (q)
        {}

        hpp::floatSeq q_;
      };
    }

    QListWidgetItem* ConfigurationListWidget::makeItem (QString name, const hpp::floatSeq& config) {
      QListWidgetItem* newItem = new ConfigItem (name, config);
      newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
      return newItem;
    }

    hpp::floatSeq& ConfigurationListWidget::getConfig (QListWidgetItem* item) {
      return static_cast<ConfigItem*>(item)->q_;
    }

    ConfigurationListWidget::ConfigurationListWidget(HppWidgetsPlugin *plugin, QWidget* parent) :
      QWidget (parent),
      plugin_ (plugin),
      ui_ (new ::Ui::ConfigurationListWidget),
      previous_ (NULL),
      main_ (gepetto::gui::MainWindow::instance()),
      basename_ ("config_"), count_ (0)
    {
      ui_->setupUi (this);

      ui_->listConfigurations->bindDeleteKey();
      ui_->listInit->setSingleItemOnly(true);

      connect (ui_->button_SaveConfig, SIGNAL (clicked()), this, SLOT (onSaveClicked()));
      connect (name(), SIGNAL (returnPressed()), SLOT (onSaveClicked()));
      connect (ui_->button_ResetGoalConfig, SIGNAL (clicked()), SLOT(resetGoalConfigs()));
      connect (list (), SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect (ui_->listGoals, SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect (ui_->listInit, SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect(ui_->listGoals, SIGNAL(configurationChanged()), SLOT(setConfigs()));
      connect(list(), SIGNAL(configurationChanged()), SLOT(setConfigs()));
      name()->setText(basename_ + QString::number(count_));
    }

    ConfigurationListWidget::~ConfigurationListWidget()
    {
      delete ui_;
    }

    ConfigurationList *ConfigurationListWidget::list() {
      return ui_->listConfigurations;
    }

    void ConfigurationListWidget::onSaveClicked ()
    {
      hpp::floatSeq const* c = plugin_->getCurrentConfig ();
      list()->addItem(makeItem(name()->text(), *c));
      count_++;
      name()->setText(basename_ + QString::number(count_));
    }

    void ConfigurationListWidget::reciveConfig (QString name_, const hpp::floatSeq& config)
    {
      list()->addItem(makeItem(name_, config));
      count_++;
      name()->setText(basename_ + QString::number(count_));
    }

    void ConfigurationListWidget::updateCurrentConfig (QListWidgetItem* current, QListWidgetItem *)
    {
      if (current != 0) {
          const hpp::floatSeq& c = getConfig(current);
          plugin_->setCurrentConfig (c);
          if (previous_ &&
              previous_ != current->listWidget()) {
              previous_->clearSelection();
              previous_->setCurrentRow(-1); // force saved index change
          }
          previous_ = current->listWidget();
        }
    }

    void ConfigurationListWidget::reinitialize(){
      resetAllConfigs();
      list()->deleteAll();
      count_ = 0;
      name()->setText(basename_ + QString::number(count_));
    }

    void ConfigurationListWidget::resetAllConfigs()
    {
      while (ui_->listGoals->count()) {
        QListWidgetItem* item = ui_->listGoals->takeItem(0);
        list()->addItem(item);
      }
      ui_->listGoals->setCurrentRow(-1);

      while (ui_->listInit->count()) {
        QListWidgetItem* item = ui_->listInit->takeItem(0);
        list()->addItem(item);
      }
      ui_->listGoals->setCurrentRow(-1);
    }

    void ConfigurationListWidget::resetGoalConfigs(bool doEmpty)
    {
      plugin_->client()->problem()->resetGoalConfigs();
      if (doEmpty) {
        while (ui_->listGoals->count()) {
          QListWidgetItem* item = ui_->listGoals->takeItem(0);

          list()->addItem(item);
        }
        ui_->listGoals->setCurrentRow(-1);
      }
    }

    void ConfigurationListWidget::setConfigs()
    {
      resetGoalConfigs(false);
      for (int i = 0; i < ui_->listGoals->count(); ++i) {
        QListWidgetItem* item = ui_->listGoals->item(i);
        plugin_->client()->problem()->addGoalConfig(getConfig(item));
      }
      if (ui_->listInit->count() == 1)
        setInitConfig(getConfig(ui_->listInit->item(0)));
    }

    void ConfigurationListWidget::setInitConfig(floatSeq& config)
    {
      plugin_->client()->problem()->setInitialConfig(config);
    }

    void ConfigurationListWidget::fetchInitAndGoalConfigs()
    {
      hpp::floatSeq_var init;
      hpp::floatSeqSeq_var goals;
      try {
        init = plugin_->client()->problem()->getInitialConfig();
        goals = plugin_->client()->problem()->getGoalConfigs();
      } catch (const hpp::Error& e) {
        qDebug () << "Could not update init and goal config:" << e.msg;
        return;
      }
      ui_->listInit->clear();
      ui_->listInit->addItem(makeItem("init", init.in()));

      ui_->listGoals->clear();
      for (CORBA::ULong i = 0; i < goals->length(); ++i)
        ui_->listGoals->addItem(makeItem(QString ("goal_%1").arg(i), goals.in()[i]));
    }

    QLineEdit *ConfigurationListWidget::name() {
      return ui_->lineEdit_configName;
    }

    QDataStream& operator>>(QDataStream& os, hpp::floatSeq& tab)
    {
      int size = 0;
      double f;
      while (!os.atEnd()) {
        tab.length(size + 1);
        os >> f;
        tab[size] = f;
        size += 1;
      }
      return os;
    }

    QDataStream& operator<<(QDataStream& os, const hpp::floatSeq& tab) {
      for (unsigned i = 0; i < tab.length(); ++i) {
        double f = tab[i];
        os << f;
      }
      return os;
    }
  } // namespace gui
} // namespace hpp
