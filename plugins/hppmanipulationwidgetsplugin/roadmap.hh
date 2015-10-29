#ifndef HPP_GUI_MANIPULATION_ROADMAP_HH
#define HPP_GUI_MANIPULATION_ROADMAP_HH

#include <hpp/gui/color-map.h>
#include <hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh>
#include <hppwidgetsplugin/roadmap.hh>

class ManipulationRoadmap : public Roadmap
{
public:
  typedef Roadmap::NodeID NodeID;
  typedef Roadmap::Color Color;

  ManipulationRoadmap (HppManipulationWidgetsPlugin* plugin_);

  virtual ~ManipulationRoadmap () {}

  virtual void initRoadmap (const std::string jointName);

  virtual void nodeColor (NodeID nodeId, Color& color);

private:
  HppManipulationWidgetsPlugin* plugin_;
  ColorMap nodeColorMap_;
};

#endif // HPP_GUI_MANIPULATION_ROADMAP_HH
