#include "hpp/gui/osgwidget.h"
#include "hpp/gui/mainwindow.h"

#include <osg/Camera>

#include <osg/DisplaySettings>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/StateSet>

#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>

#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>

#include <cassert>

#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>

#include <gepetto/viewer/urdf-parser.h>

namespace
{

QRect makeRectangle( const QPoint& first, const QPoint& second )
{
  // Relative to the first point, the second point may be in either one of the
  // four quadrants of an Euclidean coordinate system.
  //
  // We enumerate them in counter-clockwise order, starting from the lower-right
  // quadrant that corresponds to the default case:
  //
  //            |
  //       (3)  |  (4)
  //            |
  //     -------|-------
  //            |
  //       (2)  |  (1)
  //            |

  if( second.x() >= first.x() && second.y() >= first.y() )
    return QRect( first, second );
  else if( second.x() < first.x() && second.y() >= first.y() )
    return QRect( QPoint( second.x(), first.y() ), QPoint( first.x(), second.y() ) );
  else if( second.x() < first.x() && second.y() < first.y() )
    return QRect( second, first );
  else if( second.x() >= first.x() && second.y() < first.y() )
    return QRect( QPoint( first.x(), second.y() ), QPoint( second.x(), first.y() ) );

  // Should never reach that point...
  return QRect();
}

}

OSGWidget::OSGWidget( graphics::WindowsManagerPtr_t wm,
                      std::string name,
                      QWidget *parent,
                      const QGLWidget *shareWidget, Qt::WindowFlags f )
  : QGLWidget( parent, shareWidget, f )
  , graphicsWindow_()
  , wsm_ (wm)
  , wm_ ()
  , wid_ (-1)
  , viewer_ ()
  , selectionActive_( false )
  , selectionFinished_( true )
  , infoBox_ (this)
{
  osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
  osg::ref_ptr <osg::GraphicsContext::Traits> traits_ptr (new osg::GraphicsContext::Traits);
  traits_ptr->windowName = "Gepetto Viewer";
  traits_ptr->x = this->x();
  traits_ptr->y = this->y();
  traits_ptr->width = this->width();
  traits_ptr->height = this->height();
  traits_ptr->windowDecoration = false;
  traits_ptr->doubleBuffer = true;
//  traits_ptr->sharedContext = 0;
//  traits_ptr->sampleBuffers = 1;
//  traits_ptr->samples = 1;

  traits_ptr->alpha = ds->getMinimumNumAlphaBits();
  traits_ptr->stencil = ds->getMinimumNumStencilBits();
  traits_ptr->sampleBuffers = ds->getMultiSamples();
  traits_ptr->samples = ds->getNumMultiSamples();

//  graphicsWindow_ = new osgQt::GraphicsWindowQt ( traits_ptr, parent, shareWidget, f );
  graphicsWindow_ = new osgViewer::GraphicsWindowEmbedded ( traits_ptr );
  wid_ = wm->createWindow (name.c_str(), graphicsWindow_.get());
  wm_ = wsm_->getWindowManager (wid_);
  viewer_ = wm_->getViewerClone();
  osgQt::initQtWindowingSystem();
  viewer_->setUpViewerAsEmbeddedInWindow(this->x(), this->y(), this->width(), this->height());
//  viewer_->setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);
  viewer_->setThreadingModel( osgViewer::Viewer::SingleThreaded);
//  osgQt::setViewer (viewer_);
  viewer_->addEventHandler(new osgViewer::WindowSizeHandler);
  viewer_->addEventHandler(new osgViewer::StatsHandler);

  // This ensures that the widget will receive keyboard events. This focus
  // policy is not set by default. The default, Qt::NoFocus, will result in
  // keyboard events that are ignored.
  this->setFocusPolicy( Qt::StrongFocus );
  this->setMinimumSize( 100, 100 );

  // Ensures that the widget receives mouse move events even though no
  // mouse button has been pressed. We require this in order to let the
  // graphics window switch viewports properly.
  this->setMouseTracking( true );

  connect( &timer_, SIGNAL(timeout()), this, SLOT(update()));
  timer_.start (30);
}

OSGWidget::~OSGWidget()
{
}

graphics::WindowsManager::WindowID OSGWidget::windowID() const
{
  return wid_;
}

void OSGWidget::loadURDF(const QString robotName,
                         const QString urdf_file_path,
                         const QString meshDataRootDir,
                         const QString collisionOrVisual,
                         const QString linkOrObjectFrame)
{
  QByteArray rn = robotName.toLocal8Bit();
  QByteArray uf = urdf_file_path.toLocal8Bit();
  QByteArray md = meshDataRootDir.toLocal8Bit();
  wsm_->addURDF(rn.constData(),
                uf.constData(),
                md.constData());
  wsm_->addSceneToWindow(rn.constData(), wid_);
  MainWindow* w = dynamic_cast <MainWindow*> (parentWidget());
  if (w) {
      w->addBodyToTree (wsm_->getScene (robotName.toStdString()));
    }
}

void OSGWidget::paintEvent( QPaintEvent* /* paintEvent */ )
{
  this->makeCurrent();

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );

  this->paintGL();

  if( selectionActive_ && !selectionFinished_ )
  {
    painter.setPen( Qt::black );
    painter.setBrush( Qt::transparent );
    painter.drawRect( makeRectangle( selectionStart_, selectionEnd_ ) );
  }

  painter.end();

  this->swapBuffers();
  this->doneCurrent();
}

void OSGWidget::paintGL()
{
  viewer_->frame();
}

void OSGWidget::resizeGL( int width, int height )
{
  this->getEventQueue()->windowResize( this->x(), this->y(), width, height );
  graphicsWindow_->resized( this->x(), this->y(), width, height );

  this->onResize( width, height );
}

void OSGWidget::keyPressEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toAscii().data();

  if( event->key() == Qt::Key_S )
  {
    selectionActive_ = !selectionActive_;
    if (selectionActive_) infoBox_.selectionMode();
    else infoBox_.normalMode();
  }
  else if( event->key() == Qt::Key_H )
    this->onHome();
  else
    this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::keyReleaseEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toAscii().data();

  this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::mouseMoveEvent( QMouseEvent* event )
{
  // Note that we have to check the buttons mask in order to see whether the
  // left button has been pressed. A call to `button()` will only result in
  // `Qt::NoButton` for mouse move events.
  if( selectionActive_ && event->buttons() & Qt::LeftButton )
  {
    selectionEnd_ = event->pos();

    // Ensures that new paint events are created while the user moves the
    // mouse.
    this->update();
  }
  else
  {
    this->getEventQueue()->mouseMotion( static_cast<float>( event->x() ),
                                        static_cast<float>( event->y() ) );
  }
}

void OSGWidget::mousePressEvent( QMouseEvent* event )
{
  // Selection processing
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionStart_    = event->pos();
    selectionEnd_      = selectionStart_; // Deletes the old selection
    selectionFinished_ = false;           // As long as this is set, the rectangle will be drawn
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    this->getEventQueue()->mouseButtonPress( static_cast<float>( event->x() ),
                                             static_cast<float>( event->y() ),
                                             button );
    }
}

void OSGWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // Selection processing: Store end position and obtain selected objects
  // through polytope intersection.
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionEnd_      = event->pos();
    selectionFinished_ = true; // Will force the painter to stop drawing the
                               // selection rectangle

    if ((selectionStart_ - selectionEnd_).isNull()) {
        processPoint();
      } else  {
        processSelection();
      }
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    this->getEventQueue()->mouseButtonRelease( static_cast<float>( event->x() ),
                                               static_cast<float>( event->y() ),
                                               button );
  }
}

void OSGWidget::wheelEvent( QWheelEvent* event )
{
  // Ignore wheel events as long as the selection is active.
  if( selectionActive_ )
    return;

  event->accept();
  int delta = event->delta();

  osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                               : osgGA::GUIEventAdapter::SCROLL_DOWN;

  this->getEventQueue()->mouseScroll( motion );
}

bool OSGWidget::event( QEvent* event )
{
  bool handled = QGLWidget::event( event );

  // This ensures that the OSG widget is always going to be repainted after the
  // user performed some interaction. Doing this in the event handler ensures
  // that we don't forget about some event and prevents duplicate code.
  switch( event->type() )
  {
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
  case QEvent::Wheel:
    this->update();
    break;

  default:
    break;
  }

  return( handled );
}

void OSGWidget::onHome()
{
  osgViewer::ViewerBase::Views views;
  viewer_->getViews( views );

  for( std::size_t i = 0; i < views.size(); i++ )
  {
    osgViewer::View* view = views.at(i);
    view->home();
  }
}

void OSGWidget::onResize( int width, int height )
{
  osg::Camera* camera = viewer_->getCamera();
  camera->setViewport( 0, 0, this->width(), this->height() );
}

osgGA::EventQueue* OSGWidget::getEventQueue() const
{
  osgGA::EventQueue* eventQueue = graphicsWindow_->getEventQueue();

  if( eventQueue )
    return( eventQueue );
  else
    throw( std::runtime_error( "Unable to obtain valid event queue") );
}

void OSGWidget::processPoint()
{
  if (!(selectionStart_ - selectionEnd_).isNull())
    return;

  unsigned int nbSlave = viewer_->getNumSlaves();
  const osg::View::Slave& s = viewer_->getSlave(0);
  osg::ref_ptr<osg::Camera> camera = viewer_->getCamera();

  if( !camera )
    throw std::runtime_error( "Unable to obtain valid camera for selection processing" );

  double w = camera->getViewport()->width();
  double h = camera->getViewport()->height();
  if (std::abs(w - width ()) > 1)
    qDebug() << "Viewport and widgets have different width.";
  if (std::abs(h - height()) > 1)
    qDebug() << "Viewport and widgets have different height.";

  double x = selectionStart_.x();
  double y = height() - selectionStart_.y();

  osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
      new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);

  // This limits the amount of intersections that are reported by the
  // polytope intersector. Using this setting, a single drawable will
  // appear at most once while calculating intersections. This is the
  // preferred and expected behaviour.
  intersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

  osgUtil::IntersectionVisitor iv( intersector );

  camera->accept( iv );

  if( !intersector->containsIntersections() )
    return;

  osgUtil::LineSegmentIntersector::Intersections intersections = intersector->getIntersections();

  std::list <graphics::NodePtr_t> nodes;
  qDebug () << "Selected node 1:";
  for(osgUtil::LineSegmentIntersector::Intersections::iterator it = intersections.begin();
      it != intersections.end(); ++it) {
      for (size_t i = it->nodePath.size()-1; i >= 0 ; --i) {
          graphics::NodePtr_t n = wsm_->getNode(it->nodePath[i]->getName ());
          if (n) {
              nodes.push_back(n);
              qDebug () << QString::fromStdString(n->getID());
              break;
            }
        }
    }

  iv = osgUtil::IntersectionVisitor ( intersector );

  s._camera->accept( iv );

  if( !intersector->containsIntersections() )
    return;

  intersections = intersector->getIntersections();

  nodes.clear();
  qDebug () << "Selected node 2:";
  for(osgUtil::LineSegmentIntersector::Intersections::iterator it = intersections.begin();
      it != intersections.end(); ++it) {
      for (size_t i = it->nodePath.size()-1; i >= 0 ; --i) {
          graphics::NodePtr_t n = wsm_->getNode(it->nodePath[i]->getName ());
          if (n) {
              nodes.push_back(n);
              qDebug () << QString::fromStdString(n->getID());
              break;
            }
        }
    }
}

void OSGWidget::processSelection()
{
  QRect selectionRectangle = makeRectangle( selectionStart_, selectionEnd_ );
  int widgetHeight         = this->height();

  double xMin = selectionRectangle.left();
  double xMax = selectionRectangle.right();
  double yMin = widgetHeight - selectionRectangle.bottom();
  double yMax = widgetHeight - selectionRectangle.top();

  osg::ref_ptr<osgUtil::PolytopeIntersector> polytopeIntersector
      = new osgUtil::PolytopeIntersector( osgUtil::PolytopeIntersector::WINDOW,
                                          xMin, yMin,
                                          xMax, yMax );

  // This limits the amount of intersections that are reported by the
  // polytope intersector. Using this setting, a single drawable will
  // appear at most once while calculating intersections. This is the
  // preferred and expected behaviour.
  polytopeIntersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

  osgUtil::IntersectionVisitor iv( polytopeIntersector );

  osg::ref_ptr<osg::Camera> camera = viewer_->getCamera();

  if( !camera )
    throw std::runtime_error( "Unable to obtain valid camera for selection processing" );

  camera->accept( iv );

  if( !polytopeIntersector->containsIntersections() )
    return;

  osgUtil::PolytopeIntersector::Intersections intersections = polytopeIntersector->getIntersections();

  std::list <graphics::NodePtr_t> nodes;
  qDebug () << "Selected nodes:";
  for(osgUtil::PolytopeIntersector::Intersections::iterator it = intersections.begin();
      it != intersections.end(); ++it) {
      for (size_t i = it->nodePath.size()-1; i >= 0 ; --i) {
          graphics::NodePtr_t n = wsm_->getNode(it->nodePath[i]->getName ());
          if (n) {
              nodes.push_back(n);
              qDebug () << QString::fromStdString(n->getID());
              break;
            }
        }
    }
}

OSGWidget::InfoBox::InfoBox(QWidget *parent) :
  size_ (16,16),
  selection_ (QPixmap (":/osg/selection.png").scaled(size_)),
  record_ (QPixmap (":/osg/record.gif").scaled(size_)),
  label_ (parent)
{
  label_.setAutoFillBackground(true);
  label_.hide();
  label_.setGeometry(QRect (QPoint(0,0), size_));
}

void OSGWidget::InfoBox::normalMode()
{
  label_.hide();
}

void OSGWidget::InfoBox::selectionMode()
{
  label_.show();
  label_.setPixmap(selection_);
}

void OSGWidget::InfoBox::recordMode()
{
  label_.show();
  label_.setPixmap(record_);
}
