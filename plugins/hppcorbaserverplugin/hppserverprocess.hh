#ifndef HPP_GUI_HPPSERVERPROCESS_HH
#define HPP_GUI_HPPSERVERPROCESS_HH

#include <hpp/corbaserver/fwd.hh>

#include <hpp/gui/omniorb/omniorbthread.hh>

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

#endif // HPP_GUI_HPPSERVERPROCESS_HH
