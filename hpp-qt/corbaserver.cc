#include "corbaserver.hh"

OmniORBThread::OmniORBThread(int argc, const char **argv, QObject* parent) :
  QThread( parent ),
  problemSolver_ (new hpp::core::ProblemSolver),
  server_ (problemSolver_, argc, argv, true),
  interval_ (50)
{
}

OmniORBThread::~OmniORBThread()
{
}

void OmniORBThread::run()
{
  server_.startCorbaServer();
//  server_.processRequest(true);
  timerId_ = startTimer(0);
}

void OmniORBThread::timerEvent(QTimerEvent* /*event*/)
{
  serverLock_.lock ();
  killTimer(timerId_);
  timerId_ = startTimer(interval_);
  server_.processRequest(false);
  serverLock_.unlock();
}
