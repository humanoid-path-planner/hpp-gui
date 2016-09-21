#include <hppwidgetsplugin/roadmap.hh>

#include <string>
#include <sstream>

#include <QDebug>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>

#include <gepetto/gui/meta.hh>

#include <hppwidgetsplugin/conversions.hh>

namespace hpp {
  namespace gui {
    Roadmap::Roadmap(HppWidgetsPlugin *plugin):
      radius (0.01f), axisSize (1.f),
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
      wsm->addToGroup(rn, "hpp-gui");
      afterDisplay ();
      wsm->refresh();
    }

    void Roadmap::beforeDisplay ()
    {
      config_ = plugin_->client()->robot()->getCurrentConfig();
    }

    void Roadmap::afterDisplay ()
    {
      plugin_->client()->robot()->setCurrentConfig(config_.in());
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
      hpp->robot()->setCurrentConfig(n.in());
      hpp::Transform__var t; getPosition (t);
      fromHPP(t, frame);
    }

    void Roadmap::edgePositions (EdgeID edgeId, Position& start, Position& end)
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeq_var n1, n2;
      hpp::Transform__var t;
      hpp->problem()->edge(edgeId, n1.out(), n2.out());
      hpp->robot()->setCurrentConfig(n1.in());
      getPosition (t);
      fromHPP(t, start);
      hpp->robot()->setCurrentConfig(n2.in());
      getPosition (t);
      fromHPP(t, end);
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

    inline void Roadmap::getPosition (hpp::Transform__var& t) const
    {
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      if (link_) t = hpp->robot()->getLinkPosition (name_.c_str());
      else       t = hpp->robot()->getJointPosition(name_.c_str());
    }
  } // namespace gui
} // namespace hpp
