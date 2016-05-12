#include "hppcorbaserverplugin.hh"

#include <hpp/core/problem-solver.hh>
#include <hpp/corbaserver/server.hh>
#include <hppserverprocess.hh>

namespace hpp {
  namespace gui {
    HppCorbaserverPlugin::HppCorbaserverPlugin() :
      server_ (NULL)
    {
    }

    HppCorbaserverPlugin::~HppCorbaserverPlugin()
    {
      if (server_) {
        server_->wait();
        delete server_;
      }
    }

    void HppCorbaserverPlugin::init()
    {
      hpp::core::ProblemSolverPtr_t ps = hpp::core::ProblemSolver::create ();
      server_ = new gepetto::gui::CorbaServer (new HppServerProcess (
            new hpp::corbaServer::Server (ps, 0, NULL, true)));
      server_->start();
      server_->waitForInitDone();
    }

    QString HppCorbaserverPlugin::name() const
    {
      return QString ("hpp-corbaserver plugin");
    }

    Q_EXPORT_PLUGIN2 (hppcorbaserverplugin, HppCorbaserverPlugin)
  } // namespace gui
} // namespace hpp
