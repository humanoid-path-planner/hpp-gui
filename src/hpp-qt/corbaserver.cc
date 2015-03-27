#include "hpp-qt/corbaserver.hh"

OmniORBThread::OmniORBThread(int argc, const char **argv, QObject* parent) :
  QThread( parent ),
  problemSolver_ (new hpp::core::ProblemSolver),
  server_ (new hpp::corbaServer::Server (problemSolver_, argc, argv, true)),
  interrupt_ (false),
  interval_ (50)
{
}

OmniORBThread::~OmniORBThread()
{
  delete problemSolver_;
}

void OmniORBThread::requestInterruption()
{
  interrupt_ = true;
}

void OmniORBThread::run()
{
  server_->startCorbaServer();
//  server_.processRequest(true);
  timerId_ = startTimer(0);
}

void OmniORBThread::timerEvent(QTimerEvent* /*event*/)
{
  serverLock_.lock ();
  killTimer(timerId_);
  if (interrupt_) {
      delete server_;
      exit (0);
  }
  timerId_ = startTimer(interval_);
  server_->processRequest(false);
  serverLock_.unlock();
}
