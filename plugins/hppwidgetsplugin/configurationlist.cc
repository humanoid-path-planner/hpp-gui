#include <QDropEvent>
#include <QUrl>
#include <iostream>

#include "configurationlist.hh"
#include "configurationlistwidget.hh"

namespace hpp {
  namespace gui {
    ConfigurationList::ConfigurationList(QWidget* parent)
        : QListWidget(parent)
        , singleItemOnly (false)
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
        hpp::floatSeq input;
        dataStream >> input;

        if (singleItemOnly)
          clear();

        addItem(
              ConfigurationListWidget::makeItem(event->mimeData()->text(), input)
                );
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

      dataStream << ConfigurationListWidget::getConfig(item);

      QMimeData* mimeData = new QMimeData;
      mimeData->setData("application/configuration-data", itemData);

      QDrag *drag = new QDrag(this);
      drag->setMimeData(mimeData);
      mimeData->setText(item->text());
      if (drag->exec() == Qt::MoveAction) {
        if (currentRow() == row(item))
          setCurrentRow(-1);
        deleteItem(item);
        emit configurationChanged();
      }
    }

    void ConfigurationList::deleteItem(QListWidgetItem *item)
    {
      delete takeItem(row(item));
    }
  }
}
