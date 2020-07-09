//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#include <hppwidgetsplugin/roadmap.hh>

#include <string>
#include <sstream>

#include <QDebug>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>
#include <gepetto/viewer/corba/graphical-interface.hh>

#include <hppwidgetsplugin/conversions.hh>

namespace hpp {
  namespace gui {
    Roadmap::Roadmap(HppWidgetsPlugin *plugin):
      radius (0.01f), axisSize (0.05f),
      currentNodeId_ (0), currentEdgeId_ (0),
      nodeColorMap_ (0), edgeColorMap_ (0),
      plugin_ (plugin), link_ (false)
    {}

    void Roadmap::initRoadmap ()
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      int nbCC = hpp->problem()->numberConnectedComponents();
      nodeColorMap_ = gepetto::gui::ColorMap (nbCC + 10);
      edgeColorMap_ = gepetto::gui::ColorMap (nbCC + 10);

      try {
        gepetto::gui::WindowsManagerPtr_t wsm = gepetto::gui::MainWindow::instance()->osg();
        wsm->createScene (roadmapName().c_str());
        wsm->addToGroup(roadmapName(), "hpp-gui");
      } catch (const gepetto::Error&) {
        qDebug () << "Roadmap" <<
          QString::fromStdString (roadmapName ()) << "already exists.";
      }
    }

    void Roadmap::initRoadmapFromJoint(const std::string& jointName)
    {
      name_ = jointName;
      link_ = false;
      initRoadmap();
    }

    void Roadmap::initRoadmapFromBody (const std::string& bodyName)
    {
      name_ = bodyName;
      link_ = true;
      initRoadmap();
    }

    void Roadmap::displayRoadmap ()
    {
      std::size_t nbNodes = numberNodes ();
      std::size_t nbEdges = numberEdges();
      if (currentNodeId_ >= nbNodes && currentEdgeId_ >= nbEdges) return;

      std::string rn = roadmapName ();
      Color color;
      gepetto::gui::WindowsManagerPtr_t wsm = gepetto::gui::MainWindow::instance()->osg();
      if (nbNodes == 0) {
        gepetto::gui::MainWindow::instance()->logError("There is no node in the roadmap.");
        return;
      }
      beforeDisplay ();
      for (; currentNodeId_ < nbNodes; ++currentNodeId_) {
        Frame pos;
        nodePosition (currentNodeId_, pos);
        nodeColor (currentNodeId_, color);
        std::string name = nodeName (currentNodeId_);
        wsm->addXYZaxis(name, color, radius, axisSize);
        wsm->applyConfiguration(name, pos);
      }
      for (; currentEdgeId_ < nbEdges; ++currentEdgeId_) {
        Position pos1, pos2;
        edgePositions (currentEdgeId_, pos1, pos2);
        edgeColor (currentEdgeId_, color);
        std::string name = edgeName (currentEdgeId_);
        wsm->addLine(name, pos1, pos2, color);
      }
      afterDisplay ();
      wsm->refresh();
    }

    void Roadmap::beforeDisplay ()
    {
    }

    void Roadmap::afterDisplay ()
    {
    }

    std::size_t Roadmap::numberNodes ()
    {
      return plugin_->client()->problem()->numberNodes();
    }

    std::size_t Roadmap::numberEdges ()
    {
      return plugin_->client()->problem()->numberEdges();
    }

    std::string Roadmap::roadmapName ()
    {
      return "roadmap_" + name_;
    }

    std::string Roadmap::nodeName (NodeID nodeId)
    {
      std::stringstream ss;
      ss << roadmapName () << "/node" << nodeId;
      return ss.str ();
    }

    std::string Roadmap::edgeName (EdgeID edgeId)
    {
      std::stringstream ss;
      ss << roadmapName () << "/edge" << edgeId;
      return ss.str ();
    }

    void Roadmap::nodePosition (NodeID nodeId, Frame& frame)
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeq_var n = hpp->problem()->node(nodeId);
      getPosition (n.in(), frame);
    }

    void Roadmap::edgePositions (EdgeID edgeId, Position& start, Position& end)
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeq_var n1, n2;
      Frame t;
      hpp->problem()->edge(edgeId, n1.out(), n2.out());
      getPosition (n1.in(), t);
      start = t.position;
      getPosition (n2.in(), t);
      end   = t.position;
    }

    void Roadmap::nodeColor (NodeID nodeId, Color& color)
    {
      CORBA::Long iCC = plugin_->client()->problem()->connectedComponentOfNode(nodeId);
      nodeColorMap_.getColor (iCC, color);
    }

    void Roadmap::edgeColor (EdgeID edgeId, Color& color)
    {
      CORBA::Long iCC = plugin_->client()->problem()->connectedComponentOfEdge(edgeId);
      edgeColorMap_.getColor (iCC, color);
    }

    inline void Roadmap::getPosition (const hpp::floatSeq& q, Frame& t) const
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::TransformSeq_var Ts;
      // Temporary object to avoid dynamic allocation.
      // Arguments are max, length, storage, take ownership.
      char* tmps[1];
      hpp::Names_t names (1, 1, tmps, false);
      names[0] = name_.c_str();
      if (link_) Ts = hpp->robot()->getLinksPosition (q, names);
      else       Ts = hpp->robot()->getJointsPosition(q, names);
      fromHPP(Ts[0], t);
    }
  } // namespace gui
} // namespace hpp
