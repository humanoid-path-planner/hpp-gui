#include <hppmanipulationwidgetsplugin/roadmap.hh>

#include <string>
#include <sstream>

#include <hpp/gui/mainwindow.hh>
#include <hpp/gui/windows-manager.hh>

namespace hpp {
  namespace gui {
    ManipulationRoadmap::ManipulationRoadmap(HppManipulationWidgetsPlugin *plugin):
      Roadmap (plugin),
      plugin_ (plugin),
      nodeColorMap_ (0)
    {}

    void ManipulationRoadmap::initRoadmap(const std::string jointName)
    {
      Roadmap::initRoadmap (jointName);
      nodeColorMap_ = ColorMap (1 << 10);
    }

    void ManipulationRoadmap::nodeColor (NodeID nodeId, Color& color)
    {
      hpp::floatSeq_var q = plugin_->client()->problem()->node(nodeId);
      hpp::ID idGraph;
      plugin_->manipClient()->graph()->getNode(q.in(), idGraph);
      nodeColorMap_.getColor (idGraph, color);
    }
  } // namespace gui
} // namespace hpp
