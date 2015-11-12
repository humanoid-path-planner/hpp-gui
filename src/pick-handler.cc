#include "hpp/gui/pick-handler.hh"

#include <osg/io_utils>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgViewer/Viewer>

#include <iostream>
#include <boost/regex.hpp>

#include <hpp/gui/windows-manager.hh>
#include <hpp/gui/osgwidget.hh>
#include <hpp/gui/mainwindow.hh>

namespace hpp {
  namespace gui {
    PickHandler::PickHandler(WindowsManagerPtr_t wsm, OSGWidget *parent)
      : wsm_ (wsm)
      , parent_ (parent)
      , pushed_ (false)
      , lastX_ (0)
      , lastY_ (0)
    {}

    PickHandler::~PickHandler()
    {
    }

    bool PickHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
      if (ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) return false;
      if (pushed_
          && ea.getEventType() == osgGA::GUIEventAdapter::RELEASE) {
          pushed_ = false;
          if ((int)floor(lastX_ - ea.getX()+0.05) == 0
              && (int)floor(lastY_ - ea.getY() + 0.5) == 0) {
              computeIntersection(aa, ea.getX(), ea.getY());
              return true;
            }
        }
      if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH) {
          lastX_ = ea.getX();
          lastY_ = ea.getY();
          pushed_ = true;
        }

      return false;
    }

    std::list<graphics::NodePtr_t> PickHandler::computeIntersection(osgGA::GUIActionAdapter &aa,
                                                                    const float &x, const float &y)
    {
      std::list<graphics::NodePtr_t> nodes;
      osgViewer::View* viewer = dynamic_cast<osgViewer::View*>( &aa );

      if( viewer )
      {
          osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
              new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);
          intersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

          osgUtil::IntersectionVisitor iv( intersector );

          osg::Camera* camera = viewer->getCamera();

          camera->accept( iv );

          if( !intersector->containsIntersections() )
            return nodes;

          osgUtil::LineSegmentIntersector::Intersections intersections = intersector->getIntersections();

          for(osgUtil::LineSegmentIntersector::Intersections::iterator it = intersections.begin();
              it != intersections.end(); ++it) {
              for (int i = (int) it->nodePath.size()-1; i >= 0 ; --i) {
                  graphics::NodePtr_t n = wsm_->getNode(it->nodePath[i]->getName ());
                  if (n) {
                      if (boost::regex_match (n->getID(), boost::regex ("^.*_[0-9]+$")))
                        continue;
                      nodes.push_back(n);
                      break;
                    }
                }
            }
//          emit parent_->selected (nodes.front());
          MainWindow::instance()->requestSelectJointFromBodyName (nodes.front()->getID());
        }
      return nodes;
    }
  }
}
