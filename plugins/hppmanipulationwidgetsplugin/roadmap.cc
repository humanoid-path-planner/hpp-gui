//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include <hppmanipulationwidgetsplugin/roadmap.hh>

#include <string>
#include <sstream>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>

namespace hpp {
  namespace gui {
    ManipulationRoadmap::ManipulationRoadmap(HppManipulationWidgetsPlugin *plugin):
      Roadmap (plugin), plugin_ (plugin)
    {}

    void ManipulationRoadmap::initRoadmapFromJoint (const std::string& jointName)
    {
      Roadmap::initRoadmapFromJoint (jointName);
      nodeColorMap_ = gepetto::gui::ColorMap ((1 << 7) - 1);
    }

    void ManipulationRoadmap::initRoadmapFromBody  (const std::string& bodyName)
    {
      Roadmap::initRoadmapFromBody (bodyName);
      nodeColorMap_ = gepetto::gui::ColorMap ((1 << 7) - 1);
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
