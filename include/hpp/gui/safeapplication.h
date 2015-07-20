#ifndef SAFEAPPLICATION_H
#define SAFEAPPLICATION_H

#include <QApplication>

class SafeApplication : public QApplication
{
public:
  explicit SafeApplication (int& argc, char ** argv);

  virtual bool notify(QObject* receiver, QEvent* e);

signals:

public slots:

};

#endif // SAFEAPPLICATION_H
