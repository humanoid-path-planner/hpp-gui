#ifndef CONFIGURATIONLIST_HH
#define CONFIGURATIONLIST_HH

#include <QListWidget>

namespace hpp {
  namespace gui {
    class ConfigurationList : public QListWidget
    {
      Q_OBJECT

    signals:
      void configurationChanged();

    public:
      ConfigurationList(QWidget* parent);
      virtual ~ConfigurationList();

    protected:
      void dragEnterEvent(QDragEnterEvent* event);
      void dragMoveEvent(QDragMoveEvent* event);
      void dropEvent(QDropEvent *event);
      void startDrag(Qt::DropActions supportedActions);
    };
  }
}

#endif // CONFIGURATIONLIST_HH
