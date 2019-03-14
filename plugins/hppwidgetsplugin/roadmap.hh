//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#ifndef HPP_GUI_ROADMAP_HH
#define HPP_GUI_ROADMAP_HH

#include <hpp/common-idl.hh>
#include <gepetto/gui/color-map.hh>
#include <gepetto/gui/windows-manager.hh>
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace hpp {
  namespace gui {
    class Roadmap {
      public:
        typedef unsigned int NodeID;
        typedef unsigned int EdgeID;
        typedef gepetto::viewer::Configuration Frame;
        typedef osgVector3 Position;
        typedef gepetto::viewer::WindowsManager::Color_t Color;

        float radius, axisSize;

        Roadmap (HppWidgetsPlugin* plugin_);

        virtual ~Roadmap () {}

        virtual void initRoadmapFromJoint (const std::string& jointName);

        virtual void initRoadmapFromBody  (const std::string& bodyName);

        /// You can call this function several times. It will continue displaying the roadmap
        /// where it stopped.
        void displayRoadmap ();

        /// This function is called before anything else.
        /// You should save the current configuration in this function
        virtual void beforeDisplay ();

        /// This function is called after each display.
        /// You should restore the configuration you saved in the function
        virtual void afterDisplay ();

        virtual std::size_t numberNodes ();

        virtual std::size_t numberEdges ();

        virtual std::string roadmapName ();

        virtual std::string nodeName (NodeID nodeId);

        virtual std::string edgeName (EdgeID edgeId);

        virtual void nodePosition (NodeID nodeId, Frame& frame);

        virtual void edgePositions (EdgeID edgeId, Position& start, Position& end);

        virtual void nodeColor (NodeID nodeId, Color& color);

        virtual void edgeColor (EdgeID edgeId, Color& color);

      protected:
        NodeID currentNodeId_, currentEdgeId_;
        gepetto::gui::ColorMap nodeColorMap_, edgeColorMap_;

      private:
        void initRoadmap (); 
        inline void getPosition(const hpp::floatSeq& q, Frame& t) const;

        HppWidgetsPlugin* plugin_;
        std::string name_;
        bool link_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_ROADMAP_HH
