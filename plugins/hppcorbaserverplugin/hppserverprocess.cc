#include "hppserverprocess.hh"

#include <hpp/corbaserver/server.hh>
#include <hpp/corbaserver/common.hh>

namespace hpp {
  namespace gui {
    HppServerProcess::HppServerProcess(hpp::corbaServer::Server *server)
      : server_ (server)
    {}

    HppServerProcess::~HppServerProcess()
    {
      delete server_;
    }

    void HppServerProcess::init()
    {
      server_->startCorbaServer ();
      emit done ();
      ServerProcess::init();
    }

    void HppServerProcess::processRequest(bool loop)
    {
      server_->processRequest (loop);
      emit done();
    }
  } // namespace gui
} // namespace hpp
