#ifndef HPP_GUI_OSGWIDGET_HH
#define HPP_GUI_OSGWIDGET_HH

#include <QtOpenGL>
#include <QString>

#include <osg/ref_ptr>

#include <osgQt/GraphicsWindowQt>
#include <gepetto/viewer/window-manager.h>

#include <hpp/gui/fwd.hh>
#include <hpp/gui/windows-manager.hh>

namespace hpp {
  namespace gui {
    class OSGWidget : public QWidget
    {
      Q_OBJECT
      public:
        typedef std::list <graphics::NodePtr_t> NodeList;

        enum Mode {
          CAMERA_MANIPULATION,
          NODE_SELECTION,
          NODE_MOTION
        };

        OSGWidget( WindowsManagerPtr_t wm,
            std::string name,
            MainWindow* parent = 0,
            Qt::WindowFlags f = 0,
            osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::Viewer::SingleThreaded );

        virtual ~OSGWidget();

        WindowsManager::WindowID windowID () const;

signals:
        void selected (std::string name);
        void requestMotion (graphics::NodePtr_t node, graphics::Node::Arrow direction,
            float speed);

        public slots:
          void loadURDF (const QString robotName,
              const QString urdf_file_path,
              const QString meshDataRootDir);
        virtual void onHome();

        void changeMode (Mode mode);
        void selectionMode ();
        void cameraManipulationMode ();
        void addFloor();
        void attachToWindow (const std::string nodeName);

      protected:

        virtual void paintEvent( QPaintEvent* paintEvent );

      private:

        osgGA::EventQueue* getEventQueue() const;

        osg::ref_ptr<osgQt::GraphicsWindowQt> graphicsWindow_;
        WindowsManagerPtr_t wsm_;
        WindowsManager::WindowID wid_;
        graphics::WindowManagerPtr_t wm_;
        osgViewer::ViewerRefPtr viewer_;

        QPoint selectionStart_;
        QPoint selectionEnd_;
        graphics::NodePtr_t selectedNode_;

        Mode mode_;
        bool selectionFinished_;

        std::list <graphics::NodePtr_t> processPoint ();
        std::list <graphics::NodePtr_t> processSelection();

        QTimer timer_;

        struct InfoBox {
          QSize size_;
          QPixmap selection_, record_;
          QLabel* label_;

          InfoBox (QWidget* parent);
          void normalMode ();
          void selectionMode ();
          void recordMode ();
          void setMode (Mode mode);
        };
        InfoBox infoBox_;

        friend class PickHandler;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_OSGWIDGET_HH
