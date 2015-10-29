#include <hppwidgetsplugin/roadmap.hh>

#include <string>
#include <sstream>

#include <QDebug>

#include <hpp/gui/mainwindow.hh>
#include <hpp/gui/windows-manager.hh>

Roadmap::Roadmap(HppWidgetsPlugin *plugin):
  radius (0.01f), axisSize (1.f),
  currentNodeId_ (0), currentEdgeId_ (0),
  plugin_ (plugin),
  nodeColorMap_ (0), edgeColorMap_ (0)
{}

void Roadmap::initRoadmap(const std::string jointName)
{
  jointName_ = jointName;
  HppWidgetsPlugin::HppClient* hpp = plugin_->client();
  int nbCC = hpp->problem()->numberConnectedComponents();
  nodeColorMap_ = ColorMap (nbCC);
  edgeColorMap_ = ColorMap (nbCC);
  try {
    WindowsManagerPtr_t wsm = MainWindow::instance()->osg();
    wsm->createScene (roadmapName().c_str());
  } catch (const gepetto::Error&) {
    qDebug () << "Roadmap" <<
      QString::fromStdString (roadmapName ()) << "already exists.";
  }
}

void Roadmap::displayRoadmap ()
{
  std::size_t nbNodes = numberNodes ();
  std::size_t nbEdges = numberEdges();
  if (currentNodeId_ >= nbNodes && currentEdgeId_ >= nbEdges) return;

  std::string rn = roadmapName ();
  float color[4];
  WindowsManagerPtr_t wsm = MainWindow::instance()->osg();
  if (nbNodes == 0) {
      MainWindow::instance()->logError("There is no node in the roadmap.");
      return;
    }
  beforeDisplay ();
  for (; currentNodeId_ < nbNodes; ++currentNodeId_) {
      Frame pos;
      nodePosition (currentNodeId_, pos);
      nodeColor (currentNodeId_, color);
      std::string name = nodeName (currentNodeId_);
      wsm->addXYZaxis(name.c_str(), color, radius, axisSize);
      wsm->applyConfiguration(name.c_str(), pos);
    }
  for (; currentEdgeId_ < nbEdges; ++currentEdgeId_) {
      Position pos1, pos2;
      edgePositions (currentEdgeId_, pos1, pos2);
      edgeColor (currentEdgeId_, color);
      std::string name = edgeName (currentEdgeId_);
      wsm->addLine(name.c_str(), pos1, pos2, color);
    }
  wsm->addToGroup(rn.c_str(), "hpp-gui");
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
  return "roadmap_" + jointName_;
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
  hpp::Transform__var t = hpp->robot()->getLinkPosition(jointName_.c_str());
  for (int j = 0; j < 7; ++j) { frame[j] = (float)t.in()[j]; }
}

void Roadmap::edgePositions (EdgeID edgeId, Position& start, Position& end)
{
  HppWidgetsPlugin::HppClient* hpp = plugin_->client();
  hpp::floatSeq_var n1, n2;
  hpp::Transform__var t;
  hpp->problem()->edge(edgeId, n1.out(), n2.out());
  hpp->robot()->setCurrentConfig(n1.in());
  t = hpp->robot()->getLinkPosition(jointName_.c_str());
  for (int j = 0; j < 3; ++j) { start[j] = (float)t.in()[j]; }
  hpp->robot()->setCurrentConfig(n2.in());
  t = hpp->robot()->getLinkPosition(jointName_.c_str());
  for (int j = 0; j < 3; ++j) { end[j] = (float)t.in()[j]; }
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
