#include "hpp/gui/safeapplication.h"

#include <QDebug>
#include <omniORB4/CORBA.h>

SafeApplication::SafeApplication(int& argc, char** argv) :
  QApplication(argc, argv)
{
}

bool SafeApplication::notify(QObject *receiver, QEvent *e)
{
  try {
    return QApplication::notify(receiver, e);
  } catch (const std::exception& e) {
    qDebug () << e.what();
  } catch (const CORBA::Exception& e) {
    qDebug() << "CORBA Exception" << e._name() << e._rep_id();
  } catch (...) {
    qDebug() << "Unknown exception";
    qDebug() << "Catch it in SafeApplication::notify";
  }
  return false;
}
