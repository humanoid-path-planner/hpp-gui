#ifndef HPP_GUI_LEDINDICATOR_HH
#define HPP_GUI_LEDINDICATOR_HH

#include <QWidget>

namespace hpp {
  namespace gui {
    class LedIndicator : public QWidget
    {
      Q_OBJECT

      public:
        LedIndicator (QWidget *parent = 0);

signals:
        void switched (bool on);
        void mouseClickEvent ();

        public slots:
          void switchLed();
        void switchLed(bool on);

      protected:
        void paintEvent(QPaintEvent *);
        void mouseReleaseEvent(QMouseEvent* event);

      private:
        bool lit;
        const int width, height;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_LEDINDICATOR_HH
