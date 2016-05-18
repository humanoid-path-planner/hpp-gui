#include <QDropEvent>
#include <QUrl>
#include <iostream>

#include "configurationlist.hh"
#include "configurationlistwidget.hh"

namespace hpp {
  namespace gui {
    ConfigurationList::ConfigurationList(QWidget* parent)
        : QListWidget(parent)
    {
    }

    ConfigurationList::~ConfigurationList()
    {
    }

    void ConfigurationList::dragEnterEvent(QDragEnterEvent *event)
    {
      if (event->mimeData()->hasFormat("application/configuration-data")) event->accept();
      else event->ignore();
    }

    void ConfigurationList::dragMoveEvent(QDragMoveEvent *event)
    {
      if (event->mimeData()->hasFormat("application/configuration-data")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
      }
      else event->ignore();
    }

    void ConfigurationList::dropEvent(QDropEvent *event)
    {
      if (event->source() == this)
          event->ignore();
      else if (event->mimeData()->hasFormat("application/configuration-data")) {
        QByteArray data = event->mimeData()->data("application/configuration-data");
        QDataStream dataStream(&data, QIODevice::ReadOnly);
        hpp::floatSeq* input = new hpp::floatSeq;

        dataStream >> input;

        QListWidgetItem* item = new QListWidgetItem(this);
        QVariant v;
        v.setValue(input);
        item->setText(event->mimeData()->text());
        item->setData(ConfigurationListWidget::ConfigRole, v);
        addItem(item);
        event->setDropAction(Qt::MoveAction);
        event->accept();
      }
      else event->ignore();
    }

    void ConfigurationList::startDrag(Qt::DropActions /*supportedActions*/)
    {
      QListWidgetItem* item = currentItem();

      QByteArray itemData;
      QDataStream dataStream(&itemData, QIODevice::WriteOnly);
      hpp::floatSeq* fv = item->data(ConfigurationListWidget::ConfigRole).value<hpp::floatSeq*>();

      dataStream << fv;

      QMimeData* mimeData = new QMimeData;
      mimeData->setData("application/configuration-data", itemData);

      QDrag *drag = new QDrag(this);
      drag->setMimeData(mimeData);
      mimeData->setText(item->text());
      if (drag->exec() == Qt::MoveAction) {
        if (currentRow() == row(item))
          setCurrentRow(-1);
        delete takeItem(row(item));
        emit configurationChanged();
      }
    }
  }
}
