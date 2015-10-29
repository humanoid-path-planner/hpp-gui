#ifndef HPP_GUI_SAFEAPPLICATION_HH
#define HPP_GUI_SAFEAPPLICATION_HH

#include <QApplication>

class SafeApplication : public QApplication
{
public:
  explicit SafeApplication (int& argc, char ** argv);

  virtual bool notify(QObject* receiver, QEvent* e);

signals:

public slots:

};

#endif // HPP_GUI_SAFEAPPLICATION_HH
