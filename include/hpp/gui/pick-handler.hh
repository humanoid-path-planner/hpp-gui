#ifndef HPP_GUI_PICK_HANDLER_HH
#define HPP_GUI_PICK_HANDLER_HH

#include <osgGA/GUIEventHandler>
#include <gepetto/viewer/node.h>

#include <hpp/gui/fwd.hh>

namespace hpp {
  namespace gui {
    class OSGWidget;

    class PickHandler : public osgGA::GUIEventHandler
    {
    public:
      PickHandler (WindowsManagerPtr_t wsm, OSGWidget* parent);

      virtual ~PickHandler();

      virtual bool handle( const osgGA::GUIEventAdapter&  ea,
                                 osgGA::GUIActionAdapter& aa );

    private:
      std::list <graphics::NodePtr_t> computeIntersection (osgGA::GUIActionAdapter& aa,
                                                           const float& x, const float& y);

      WindowsManagerPtr_t wsm_;
      OSGWidget* parent_;
      bool pushed_;
      float lastX_, lastY_;
    };
  }
}

#endif // HPP_GUI_PICK_HANDLER_HH
