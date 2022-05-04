//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#ifndef HPP_GUI_HPPSERVERPROCESS_HH
#define HPP_GUI_HPPSERVERPROCESS_HH

#include <gepetto/gui/omniorb/omniorbthread.hh>
#include <hpp/corbaserver/fwd.hh>

namespace hpp {
namespace gui {
class HppServerProcess : public gepetto::gui::ServerProcess {
  Q_OBJECT

 public:
  HppServerProcess(hpp::corbaServer::Server* server_);

  ~HppServerProcess();

 public slots:
  void init();
  void processRequest(bool loop);

 private:
  hpp::corbaServer::Server* server_;
};
}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_HPPSERVERPROCESS_HH
