#ifndef OSGWIDGET_H
#define OSGWIDGET_H

#include <QtOpenGL>
#include <QString>

#include <osg/ref_ptr>

#include <osgQt/GraphicsWindowQt>
#include <gepetto/viewer/window-manager.h>

#include <hpp/gui/fwd.h>
#include <hpp/gui/windows-manager.h>

class OSGWidget : public QGLWidget
{
  Q_OBJECT
public:
  OSGWidget( WindowsManagerPtr_t wm,
             std::string name,
             QWidget* parent = 0,
             const QGLWidget* shareWidget = 0,
             Qt::WindowFlags f = 0 );

  virtual ~OSGWidget();

  WindowsManager::WindowID windowID () const;

public slots:
  void loadURDF (const QString robotName,
                 const QString urdf_file_path,
                 const QString meshDataRootDir,
                 const QString collisionOrVisual="visual",
                 const QString linkOrObjectFrame="link");
  virtual void onHome();

protected:

  virtual void paintEvent( QPaintEvent* paintEvent );
  virtual void paintGL();
  virtual void resizeGL( int width, int height );

  virtual void keyPressEvent( QKeyEvent* event );
  virtual void keyReleaseEvent( QKeyEvent* event );

  virtual void mouseMoveEvent( QMouseEvent* event );
  virtual void mousePressEvent( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent( QWheelEvent* event );

  virtual bool event( QEvent* event );

private:

  virtual void onResize( int width, int height );

  osgGA::EventQueue* getEventQueue() const;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow_;
//  osg::ref_ptr<osgQt::GraphicsWindowQt> graphicsWindow_;
  WindowsManagerPtr_t wsm_;
  WindowsManager::WindowID wid_;
  graphics::WindowManagerPtr_t wm_;
  osgViewer::ViewerRefPtr viewer_;

  QPoint selectionStart_;
  QPoint selectionEnd_;

  bool selectionActive_;
  bool selectionFinished_;

  void processPoint ();
  void processSelection();

  QTimer timer_;

  struct InfoBox {
    QSize size_;
    QPixmap selection_, record_;
    QLabel label_;

    InfoBox (QWidget* parent);
    void normalMode ();
    void selectionMode ();
    void recordMode ();
  };
  InfoBox infoBox_;
};

#endif // OSGWIDGET_H
