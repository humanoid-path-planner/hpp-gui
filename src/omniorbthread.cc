#include "hpp/gui/omniorb/omniorbthread.h"

#include <hpp/corbaserver/server.hh>
#include <hpp/corbaserver/common.hh>
#include <gepetto/viewer/corba/server.hh>
#include <QDebug>

int WorkItem::idGlobal = 0;

HppServerProcess::HppServerProcess(hpp::corbaServer::Server *server)
  : server_ (server)
{}

void HppServerProcess::init()
{
  server_->startCorbaServer ();
  emit done ();
}

void HppServerProcess::processRequest(bool loop)
{
  server_->processRequest (loop);
  emit done();
}

ViewerServerProcess::ViewerServerProcess (graphics::corbaServer::Server *server)
  : server_ (server)
{}

void ViewerServerProcess::init()
{
  server_->startCorbaServer ();
  emit done ();
}

void ViewerServerProcess::processRequest(bool loop)
{
  server_->processRequest (loop);
  emit done();
}

CorbaServer::CorbaServer (ServerProcess* process) :
  QObject (), control_ (process), worker_ (), timerId_ (-1), interval_ (100)
{
  connect (this, SIGNAL (process(bool)), control_, SLOT (processRequest (bool)));
  connect (control_, SIGNAL (done()), this, SLOT (processed()));
  connect (&worker_, SIGNAL (started()), control_, SLOT (init()));
  control_->moveToThread(&worker_);
}

CorbaServer::~CorbaServer()
{
  if (control_)
    delete control_;
}

void CorbaServer::wait ()
{
  worker_.quit ();
  worker_.wait(200);
  if (worker_.isRunning()) {
      worker_.terminate();
      worker_.wait();
    }
}

void CorbaServer::start()
{
  worker_.start();
}

void CorbaServer::timerEvent(QTimerEvent* event)
{
  assert (event->timerId () == timerId_);
  emit process (false);
  killTimer(timerId_);
}

void CorbaServer::processed()
{
  timerId_ = startTimer(interval_);
}

BackgroundQueue::BackgroundQueue(QObject *parent) :
  QObject (parent)
{
}

void BackgroundQueue::perform(WorkItem *item)
{
  try {
    item->performWork();
    emit done (item->id ());
  } catch (std::exception& e) {
    emit failed (item->id(), QString (e.what()));
  } catch (hpp::Error& e) {
    emit failed (item->id(), QString (e.msg));
  }
  delete item;
}
