#include "hpp/gui/safeapplication.h"

#include <QDebug>
#include <omniORB4/CORBA.h>
#include <hpp/gui/mainwindow.h>
#include <hpp/gui/plugin-interface.h>

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
  } catch (const CORBA::TRANSIENT& e) {
    MainWindow::instance()->logError(QString ("CORBA Exception %1 - %2.\nYou may need to reset the connections (see Tools menu)").arg(e._name()).arg(e._rep_id()));
  } catch (const CORBA::Exception& e) {
    bool handled = false;
    foreach (CorbaInterface* errorHandler, MainWindow::instance()->pluginManager()->get <CorbaInterface>()) {
        if (errorHandler->corbaException (0, e)) {
            handled = true;
            break;
          }
      }
    if (!handled)
      MainWindow::instance ()->logError(QString ("Unhandled CORBA Exception %1 - %2").arg(e._name()).arg(e._rep_id()));
  } catch (...) {
    qDebug() << "Unknown exception";
    qDebug() << "Catch it in SafeApplication::notify";
  }
  return false;
}
