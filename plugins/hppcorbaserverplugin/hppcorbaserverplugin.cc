#include "hppcorbaserverplugin.hh"

#include <hpp/core/problem-solver.hh>
#include <hpp/corbaserver/server.hh>
#include <hppserverprocess.hh>

/// Plugin to emulate a corbaserver for hpp-core

/// \namespace hpp
/// namespace that encapsulate all the softwares of humanoid-path-planner
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

    /// \fn void HppCorbaserver::init()
    /// Init the plugin
    void HppCorbaserverPlugin::init()
    {
      hpp::core::ProblemSolverPtr_t ps = hpp::core::ProblemSolver::create ();
      server_ = new gepetto::gui::CorbaServer (new HppServerProcess (
            new hpp::corbaServer::Server (ps, 0, NULL, true)));
      server_->start();
      server_->waitForInitDone();
    }

    /// \fn QString HppCorbaserverPlugin::name() const
    /// Return the name of the plugin
    /// \return name of the plugin
    QString HppCorbaserverPlugin::name() const
    {
      return QString ("hpp-corbaserver plugin");
    }

    Q_EXPORT_PLUGIN2 (hppcorbaserverplugin, HppCorbaserverPlugin)
  } // namespace gui
} // namespace hpp
