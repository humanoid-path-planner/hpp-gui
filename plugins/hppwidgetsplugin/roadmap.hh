#ifndef HPP_GUI_ROADMAP_HH
#define HPP_GUI_ROADMAP_HH

#include <hpp/corbaserver/common.hh>
#include <hpp/gui/color-map.hh>
#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace hpp {
  namespace gui {
    class Roadmap {
      public:
        typedef unsigned int NodeID;
        typedef unsigned int EdgeID;
        typedef float Frame[7];
        typedef float Position[3];
        typedef float Color[4];

        float radius, axisSize;

        Roadmap (HppWidgetsPlugin* plugin_);

        virtual ~Roadmap () {}

        virtual void initRoadmap (const std::string jointName);

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
        std::size_t currentNodeId_, currentEdgeId_;

      private:
        HppWidgetsPlugin* plugin_;
        std::string jointName_;
        hpp::floatSeq_var config_;

        ColorMap nodeColorMap_, edgeColorMap_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_ROADMAP_HH
