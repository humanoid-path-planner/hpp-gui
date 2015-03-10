#ifndef CORBASERVER_HH
#define CORBASERVER_HH

#include <QThread>
#include <QMutex>

#include <hpp/corbaserver/server.hh>
#include <omniORB4/CORBA.h>

class OmniORBThread : public QThread
{
  Q_OBJECT

public:
  OmniORBThread(int argc, const char **argv, QObject *parent = 0);
  ~OmniORBThread ();

protected:
  void run ();
  void timerEvent(QTimerEvent *event);

  hpp::core::ProblemSolverPtr_t problemSolver_;
  hpp::corbaServer::Server server_;

  int interval_;
  int timerId_;
  QMutex serverLock_;
};

#endif // CORBASERVER_HH
