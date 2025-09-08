//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#include "hppserverprocess.hh"

#include <hpp/corbaserver/common.hh>
#include <hpp/corbaserver/server.hh>

namespace hpp {
namespace gui {
HppServerProcess::HppServerProcess(hpp::corbaServer::Server* server)
    : server_(server) {}

HppServerProcess::~HppServerProcess() { delete server_; }

void HppServerProcess::init() {
  server_->startCorbaServer();
  emit done();
  gepetto::gui::ServerProcess::init();
}

void HppServerProcess::processRequest(bool loop) {
  server_->processRequest(loop);
  emit done();
}
}  // namespace gui
}  // namespace hpp
