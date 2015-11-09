#ifndef HPP_GUI_SAFEAPPLICATION_HH
#define HPP_GUI_SAFEAPPLICATION_HH

#include <QApplication>

namespace hpp {
  namespace gui {
    class SafeApplication : public QApplication
    {
      Q_OBJECT

      public:
        explicit SafeApplication (int& argc, char ** argv);

        virtual bool notify(QObject* receiver, QEvent* e);

      signals:
        void log (QString text);
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_SAFEAPPLICATION_HH
