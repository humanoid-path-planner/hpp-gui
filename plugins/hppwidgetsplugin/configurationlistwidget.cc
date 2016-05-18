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
      previous_ (NULL),
      main_ (gepetto::gui::MainWindow::instance()),
      basename_ ("config_"), count_ (0)
    {
      ui_->setupUi (this);

      connect (ui_->button_SaveConfig, SIGNAL (clicked()), this, SLOT (onSaveClicked()));
      connect (ui_->button_ResetGoalConfig, SIGNAL (clicked()), SLOT(resetGoalConfigs()));
      connect (list (), SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect (ui_->listGoals, SIGNAL (currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
          this, SLOT (updateCurrentConfig(QListWidgetItem*,QListWidgetItem*)));
      connect (list (), SIGNAL (doubleClicked(const QModelIndex&)),
          this, SLOT (onDoubleClick(const QModelIndex&)));
      connect (ui_->listGoals, SIGNAL (doubleClicked(const QModelIndex&)),
          this, SLOT (onDoubleClick(const QModelIndex&)));
      connect(ui_->listGoals, SIGNAL(configurationChanged()), SLOT(setConfigs()));
      connect(list(), SIGNAL(configurationChanged()), SLOT(setConfigs()));
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
          if (previous_ &&
              previous_ != current->listWidget()) {
              previous_->clearSelection();
              previous_->setCurrentRow(-1); // force saved index change
          }
          previous_ = current->listWidget();
        }
    }

    void ConfigurationListWidget::onDoubleClick (const QModelIndex& pos)
    {
      QListWidgetItem* item = list()->item(pos.row());
      if (!item) {
        item = ui_->listGoals->item(pos.row());
        if (!item)
          return;
      }
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

        plugin_->client()->problem()->addGoalConfig(*(item->data(ConfigRole).value<hpp::floatSeq*>()));
      }
    }

    void ConfigurationListWidget::setInitConfig(floatSeq *config)
    {
      plugin_->client()->problem()->setInitialConfig(*config);
    }

    QLineEdit *ConfigurationListWidget::name() {
      return ui_->lineEdit_configName;
    }

    DropInitial::DropInitial(QWidget *parent)
      : QLabel(parent)
    {
      ConfigurationListWidget* cl = dynamic_cast<ConfigurationListWidget*>(parent);

      list_ = cl->list();
      setStyleSheet("QLabel { background-color: white; border: 1px solid black }");
      timer_ = new QBasicTimer();
      alreadyReleased_ = false;
    }

    DropInitial::~DropInitial()
    {
      delete timer_;
    }

    void DropInitial::dragEnterEvent(QDragEnterEvent *event)
    {
      if (event->mimeData()->hasFormat("application/configuration-data")) event->accept();
      else event->ignore();
    }

    void DropInitial::dragMoveEvent(QDragMoveEvent *event)
    {
      if (event->mimeData()->hasFormat("application/configuration-data")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
      }
      else event->ignore();
    }

    void DropInitial::mouseReleaseEvent(QMouseEvent* /*event*/)
    {
      if (alreadyReleased_) {
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
            setText(newName->text());
          }
          alreadyReleased_ = false;
          timer_->stop();
      }
      else alreadyReleased_ = true;
    }

    void DropInitial::mousePressEvent(QMouseEvent* /*event*/)
    {
      if (!timer_->isActive()) {
        timer_->start(150, this);
      }
    }

    void DropInitial::timerEvent(QTimerEvent *)
    {
      timer_->stop();
      alreadyReleased_ = false;
      if (!alreadyReleased_ && text() != "") {
        QByteArray data;
        QDataStream dataStream(&data, QIODevice::WriteOnly);

        dataStream << fs_;

        QMimeData* mime = new QMimeData;
        mime->setData("application/configuration-data", data);
        mime->setText(text());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mime);
        if (drag->exec() == Qt::MoveAction) {
          setText("");
          fs_ = NULL;
        }
      }
    }

    hpp::floatSeq* DropInitial::getConfig() const
    {
      return fs_;
    }

    void DropInitial::dropEvent(QDropEvent *event)
    {
      if (event->source() == this) {
        event->ignore();
      }
      else if (event->mimeData()->hasFormat("application/configuration-data")) {
        if (text() != "") {
          QListWidgetItem* item = new QListWidgetItem(list_);
          QVariant v;

          v.setValue(fs_);
          item->setText(text());
          item->setData(ConfigurationListWidget::ConfigRole, v);
          list_->addItem(item);
        }
        QByteArray data = event->mimeData()->data("application/configuration-data");
        QDataStream dataStream(&data, QIODevice::ReadOnly);
        fs_ = new hpp::floatSeq;

        dataStream >> fs_;
        setText(event->mimeData()->text());
        event->setDropAction(Qt::MoveAction);
        event->accept();
        ConfigurationListWidget* clw = dynamic_cast<ConfigurationListWidget*>(parent());

        clw->setInitConfig(fs_);
      }
      else event->ignore();
    }

    QDataStream& operator>>(QDataStream& os, hpp::floatSeq*& tab)
    {
      int size = 0;
      tab->length(1);
      double f;
      while (!os.atEnd()) {
        os >> f;
        (*tab)[size] = f;
        if (!os.atEnd()) {
          size += 1;
          tab->length(size + 1);
        }
      }
      return os;
    }

    QDataStream& operator<<(QDataStream& os, hpp::floatSeq*& tab) {
      double f;
      for (unsigned i = 0; i < tab->length(); ++i) {
        f = (*tab)[i];
        os << f;
      }
      return os;
    }
  } // namespace gui
} // namespace hpp
