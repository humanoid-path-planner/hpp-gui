#ifndef HPP_GUI_PICK_HANDLER_HH
#define HPP_GUI_PICK_HANDLER_HH

#include <osgGA/GUIEventHandler>
#include <gepetto/viewer/node.h>

#include <QObject>
#include <QModelIndex>

#include <hpp/gui/fwd.hh>

namespace hpp {
  namespace gui {
    class OSGWidget;

    class PickHandler : public QObject, public osgGA::GUIEventHandler
    {
      Q_OBJECT

    public:
      PickHandler (WindowsManagerPtr_t wsm, OSGWidget* parent);

      virtual ~PickHandler();

      virtual bool handle( const osgGA::GUIEventAdapter&  ea,
                                 osgGA::GUIActionAdapter& aa );

      void select (graphics::NodePtr_t node);

    private slots:
      void bodyTreeCurrentChanged (const QModelIndex &current,
          const QModelIndex &previous);

    private:
      std::list <graphics::NodePtr_t> computeIntersection (osgGA::GUIActionAdapter& aa,
                                                           const float& x, const float& y);

      WindowsManagerPtr_t wsm_;
      OSGWidget* parent_;
      graphics::NodePtr_t last_;
      bool pushed_;
      float lastX_, lastY_;
    };
  }
}

#endif // HPP_GUI_PICK_HANDLER_HH
