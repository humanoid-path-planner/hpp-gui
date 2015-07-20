#ifndef HPPSERVERPROCESS_HH
#define HPPSERVERPROCESS_HH

#include <hpp/corbaserver/fwd.hh>

#include <hpp/gui/omniorb/omniorbthread.h>

class HppServerProcess : public ServerProcess
{
  Q_OBJECT

public:
  HppServerProcess (hpp::corbaServer::Server* server_);

  ~HppServerProcess ();

public slots:
  void init ();
  void processRequest (bool loop);

private:
  hpp::corbaServer::Server* server_;
};

#endif // HPPSERVERPROCESS_HH
