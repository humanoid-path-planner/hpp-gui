#include "hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh"

#include "hppmanipulationwidgetsplugin/roadmap.hh"
#include "gepetto/gui/mainwindow.hh"
#include "gepetto/gui/windows-manager.hh"

#include "hppwidgetsplugin/conversions.hh"
#include "hppwidgetsplugin/jointtreewidget.hh"
#include "hppmanipulationwidgetsplugin/linkwidget.hh"
#include "hppmanipulationwidgetsplugin/manipulationconstraintwidget.hh"
#include "hppwidgetsplugin/twojointsconstraint.hh"
#include "hppmanipulationwidgetsplugin/manipulationlockedjoint.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
    HppManipulationWidgetsPlugin::HppManipulationWidgetsPlugin() :
      HppWidgetsPlugin (),
      hpp_ (NULL),
      toolBar_ (NULL),
      tw_ (NULL)
    {
    }

    HppManipulationWidgetsPlugin::~HppManipulationWidgetsPlugin()
    {
      if (tw_) tw_->deleteLater();
    }

    void HppManipulationWidgetsPlugin::init()
    {
      HppWidgetsPlugin::init ();
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();

      toolBar_ = gepetto::gui::MainWindow::instance()->addToolBar("Manipulation tools");
      QAction* drawRContact = new QAction ("Draw robot contacts",toolBar_);
      QAction* drawEContact = new QAction ("Draw environment contacts",toolBar_);
      toolBar_->addAction (drawRContact);
      toolBar_->addAction (drawEContact);
      connect (drawRContact, SIGNAL(triggered()), SLOT (drawRobotContacts()));
      connect (drawEContact, SIGNAL(triggered()), SLOT (drawEnvironmentContacts()));
      QAction* drawHFrame = new QAction ("Draw handles frame",toolBar_);
      QAction* drawGFrame = new QAction ("Draw grippers frame",toolBar_);
      toolBar_->addAction (drawHFrame);
      toolBar_->addAction (drawGFrame);
      connect (drawHFrame, SIGNAL(triggered()), SLOT (drawHandlesFrame()));
      connect (drawGFrame, SIGNAL(triggered()), SLOT (drawGrippersFrame()));
      QAction* autoBuildGraph = new QAction ("Autobuild constraint graph",toolBar_);
      toolBar_->addAction (autoBuildGraph);
      connect(autoBuildGraph, SIGNAL(triggered()), SLOT(autoBuildGraph()));

      connect(&main->worker(), SIGNAL(done(int)), SLOT(handleWorkerDone(int)));
      main->registerSlot("autoBuildGraph", this);
      main->registerSlot("drawRobotContacts", this);
      main->registerSlot("drawEnvironmentsContacts", this);
    }

    QString HppManipulationWidgetsPlugin::name() const
    {
      return QString ("Widgets for hpp-manipulation-corba");
    }

    void HppManipulationWidgetsPlugin::loadRobotModel(gepetto::gui::DialogLoadRobot::RobotDefinition rd)
    {
      try {
	client ()->robot ()->getCurrentConfig();
      } catch (hpp::Error const& e) {
	hpp_->robot ()->create (gepetto::gui::Traits<QString>::to_corba("composite").in());
      }
      hpp_->robot ()->insertRobotModel (gepetto::gui::Traits<QString>::to_corba(rd.robotName_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.rootJointType_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.package_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.modelName_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.urdfSuf_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.srdfSuf_).in());
      // This is already done in requestRefresh
      // jointTreeWidget_->reload();
      applyCurrentConfiguration();
      gepetto::gui::MainWindow::instance()->requestRefresh();
      emit logSuccess ("Robot " + rd.name_ + " loaded");
    }

    void HppManipulationWidgetsPlugin::loadEnvironmentModel(gepetto::gui::DialogLoadEnvironment::EnvironmentDefinition ed)
    {
      try {
	client ()->robot ()->getCurrentConfig();
      } catch (hpp::Error const& e) {
	hpp_->robot ()->create (gepetto::gui::Traits<QString>::to_corba("composite").in());
      }
      hpp_->robot ()-> loadEnvironmentModel(gepetto::gui::Traits<QString>::to_corba(ed.package_).in(),
					    gepetto::gui::Traits<QString>::to_corba(ed.urdfFilename_).in(),
					    gepetto::gui::Traits<QString>::to_corba(ed.urdfSuf_).in(),
					    gepetto::gui::Traits<QString>::to_corba(ed.srdfSuf_).in(),
					    gepetto::gui::Traits<QString>::to_corba(ed.name_ + "/").in());
      HppWidgetsPlugin::computeObjectPosition();
      gepetto::gui::MainWindow::instance()->requestRefresh();
      emit logSuccess ("Environment " + ed.name_ + " loaded");
   }

    std::string HppManipulationWidgetsPlugin::getBodyFromJoint(const std::string &jointName) const
    {
      /// TODO: fix this
      return HppWidgetsPlugin::getBodyFromJoint (jointName);
    }

    void HppManipulationWidgetsPlugin::openConnection()
    {
      HppWidgetsPlugin::openConnection();
      hpp_ = new HppManipClient (0,0);
      QByteArray iiop = getHppIIOPurl ().toAscii();
      hpp_->connect (iiop.constData ());
    }

    void HppManipulationWidgetsPlugin::closeConnection()
    {
      HppWidgetsPlugin::closeConnection();
      if (hpp_) delete hpp_;
      hpp_ = NULL;
    }

    HppManipulationWidgetsPlugin::HppManipClient *HppManipulationWidgetsPlugin::manipClient() const
    {
      return hpp_;
    }

    void HppManipulationWidgetsPlugin::updateRobotJoints(const QString robotName)
    {
      Q_UNUSED(robotName)
      hpp::Names_t_var joints = client()->robot()->getAllJointNames ();
      for (size_t i = 0; i < joints->length (); ++i) {
        const char* jname = joints[(ULong) i];
        hpp::Names_t_var lnames = client()->robot()->getLinkNames (jname);
        jointMap_[jname] = JointElement(jname, "", lnames, 0, true);
      }
    }

    Roadmap *HppManipulationWidgetsPlugin::createRoadmap(const std::string &jointName)
    {
      ManipulationRoadmap* r = new ManipulationRoadmap(this);
      r->initRoadmapFromJoint(jointName);
      return r;
    }

    void HppManipulationWidgetsPlugin::drawContactSurface(const std::string& name,
							  hpp::intSeq_var& indexes,
							  hpp::floatSeqSeq_var& points,
							  CORBA::ULong j,
							  double epsilon)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      const float color[] = {0, 1, 0, 1};
      QVector3D norm(0, 0, 0);
      CORBA::Long iPts = (j == 0) ? 0 : indexes[j - 1];
      gepetto::corbaserver::PositionSeq ps; ps.length (indexes[j] - iPts);
      if (ps.length() > 3) {
	QVector3D a(points[iPts][0] - points[iPts + 1][0],
		    points[iPts][1] - points[iPts + 1][1],
		    points[iPts][2] - points[iPts + 1][2]);
	QVector3D b(points[iPts + 1][0] - points[iPts + 2][0],
		    points[iPts + 1][1] - points[iPts + 2][1],
		    points[iPts + 1][2] - points[iPts + 2][2]);
	QVector3D c = QVector3D::crossProduct(a, b);
	if (c.length() > 0.00001 || c.length() < -0.00001)
	  norm = c / c.length();
      }
      for (CORBA::Long k = iPts; k < indexes[j]; ++k) {
	ps[k - iPts][0] = (float)points[k][0] + norm.x() * epsilon;
	ps[k - iPts][1] = (float)points[k][1] + norm.y() * epsilon;
	ps[k - iPts][2] = (float)points[k][2] + norm.z() * epsilon;
      }
      main->osg()->addCurve (name.c_str(), ps, color);
      main->osg()->setCurveMode (name.c_str(), GL_POLYGON);
    }

    void HppManipulationWidgetsPlugin::drawRobotContacts()
    {
      hpp::Names_t_var rcs = hpp_->problem()->getRobotContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getRobotContact (
            rcs[i], indexes.out(), points.out());
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Create group
          std::string target = createJointGroup(std::string(cjs[j]));

          /// Add the contacts
          std::stringstream ssname;
          ssname <<  target << "/contact_"
		 << escapeJointName(std::string (rcs[i])) << "_" << j << "_";
          std::string name = ssname.str ();
	  drawContactSurface(name, indexes, points, j);
        }
      }
      gepetto::gui::MainWindow::instance()->requestRefresh();
    }

    void HppManipulationWidgetsPlugin::drawEnvironmentContacts()
    {
      hpp::Names_t_var rcs = hpp_->problem()->getEnvironmentContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getEnvironmentContact (
            rcs[i], indexes.out(), points.out());
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Add the contacts
          std::stringstream ssname;
          ssname << "hpp-gui/contact_"
            << escapeJointName(std::string (rcs[i])) << "_" << j << "_";
          std::string name = ssname.str ();
	  drawContactSurface(name, indexes, points, j);
        }
      }
      gepetto::gui::MainWindow::instance()->requestRefresh();
    }

    void HppManipulationWidgetsPlugin::drawHandlesFrame()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getAvailable("handle");
      hpp::Transform__var t (new Transform_);
      graphics::WindowsManager::value_type t_gv[7];
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        const std::string jn =
          hpp_->robot()->getHandlePositionInJoint (rcs[i],t.out());
        std::string groupName = createJointGroup (jn.c_str());
        std::string hn = "handle_" + escapeJointName (std::string(rcs[i]));
        fromHPP(t, t_gv);
        main->osg()->addXYZaxis (hn.c_str(), color, 0.005f, 1.f);
        main->osg()->applyConfiguration (hn.c_str(), t_gv);
        main->osg()->addToGroup (hn.c_str(), groupName.c_str());
      }
      main->osg()->refresh();
      gepetto::gui::MainWindow::instance()->requestRefresh();
    }

    void HppManipulationWidgetsPlugin::drawGrippersFrame()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getAvailable("gripper");
      hpp::Transform__var t (new Transform_);
      graphics::WindowsManager::value_type t_gv[7];
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        const std::string jn =
          hpp_->robot()->getGripperPositionInJoint (rcs[i],t.out());
        std::string groupName = createJointGroup (jn.c_str());
        std::string hn = "gripper_" + escapeJointName (std::string(rcs[i]));
        fromHPP(t, t_gv);
        main->osg()->addXYZaxis (hn.c_str(), color, 0.005f, 1.f);
        main->osg()->applyConfiguration (hn.c_str(), t_gv);
        main->osg()->addToGroup (hn.c_str(), groupName.c_str());
      }
      main->osg()->refresh();
      gepetto::gui::MainWindow::instance()->requestRefresh();
    }

    HppManipulationWidgetsPlugin::NamesPair
    HppManipulationWidgetsPlugin::convertMap(std::map<std::string,
					     std::list<std::string> >& mapNames)
    {
      HppManipulationWidgetsPlugin::NamesPair names;
      int i = 0;
      int j;

      names.first.length(mapNames.size());
      names.second.length(mapNames.size());
      for (HppManipulationWidgetsPlugin::MapNames::iterator itMap = mapNames.begin();
	   itMap != mapNames.end(); itMap++, i++){
	names.first[i] = (*itMap).first.c_str();
	names.second[i].length((*itMap).second.size());
        j = 0;
	for (std::list<std::string>::iterator itList = (*itMap).second.begin();
	     itList != (*itMap).second.end(); itList++, j++) {
	  names.second[i][j] = (*itList).c_str();
	}
      }
      return names;
    }

    hpp::Names_t_var HppManipulationWidgetsPlugin::convertToNames(const QList<QListWidgetItem *>& l)
    {
      hpp::Names_t_var cl = new hpp::Names_t;
      cl->length(l.count());
      for (int i = 0; i < l.count(); i++) {
        cl[i]= l[i]->text().toStdString().c_str();
      }
      return cl;
    }

    void HppManipulationWidgetsPlugin::mergeShapes(MapNames &handles, MapNames &shapes)
    {
      MapNames::iterator itH = handles.begin();
      MapNames::iterator itS = shapes.begin();
      while (itH != handles.end())
      {
        if (itS == shapes.end() || (*itH).first != (*itS).first) {
          itS = shapes.insert(itS, std::make_pair((*itH).first, std::list<std::string>()));
        }
        else
          ++itS;
        itH++;
      }
      while (itS != shapes.end())
      {
        handles.insert(handles.end(), std::make_pair((*itS).first, std::list<std::string>()));
        itS++;
      }
    }

    HppManipulationWidgetsPlugin::MapNames HppManipulationWidgetsPlugin::getObjects()
    {
      HppManipulationWidgetsPlugin::MapNames map;
      hpp::Names_t_var handles = manipClient()->problem()->getAvailable("handle");
      hpp::Names_t_var surfaces = manipClient()->problem()->getAvailable("robotcontact");

      for (unsigned i = 0; i < handles->length(); ++i) {
        std::string name(handles[i].in());
	size_t pos = name.find_first_of("/");
	std::string object = name.substr(0, pos);

	if (map.find(object) == map.end()) {
	  map[object] = std::list<std::string>();
	}
      }
      for (unsigned i = 0; i < surfaces->length(); ++i) {
        std::string name(surfaces[i].in());
	size_t pos = name.find_first_of("/");
	std::string object = name.substr(0, pos);

	if (map.find(object) == map.end()) {
	  map[object] = std::list<std::string>();
	}
      }
      return map;
    }

    void HppManipulationWidgetsPlugin::fillMap(HppManipulationWidgetsPlugin::MapNames& map,
					       const QList<QListWidgetItem*>& list)
    {
      for (int i = 0; i < list.count(); i++) {
        std::string name(list[i]->text().toStdString());
	size_t pos = name.find_first_of("/");
	std::string object = name.substr(0, pos);

	if (map.find(object) != map.end()) {
	  map[object].push_back(name);
	}
      }
    }

    void HppManipulationWidgetsPlugin::buildGraph()
    {
      QListWidget* l = dynamic_cast<QListWidget*>(tw_->widget(0));
      HppManipulationWidgetsPlugin::MapNames handlesMap = getObjects();
      HppManipulationWidgetsPlugin::MapNames shapesMap = getObjects();
      hpp::Names_t_var grippers = convertToNames(l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(1)->layout()->itemAt(1)->widget());
      fillMap(handlesMap, l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(2));
      fillMap(shapesMap, l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(3));
      hpp::Names_t_var envNames = convertToNames(l->selectedItems());
      hpp::corbaserver::manipulation::Rules_var rules = dynamic_cast<LinkWidget *>(tw_->widget(4))->getRules();

      HppManipulationWidgetsPlugin::NamesPair handles = convertMap(handlesMap);
      HppManipulationWidgetsPlugin::NamesPair shapes = convertMap(shapesMap);
      hpp_->graph ()->createGraph("constraints");
      gepetto::gui::WorkItem* item = new gepetto::gui::WorkItem_7<hpp::corbaserver::manipulation::_objref_Graph, hpp::intSeq*,
              const char *, const hpp::Names_t&, const hpp::Names_t&, const hpp::corbaserver::manipulation::Namess_t&, const hpp::corbaserver::manipulation::Namess_t&,
              const hpp::Names_t&, const hpp::corbaserver::manipulation::Rules&>
              (hpp_->graph().in(), &hpp::corbaserver::manipulation::_objref_Graph::autoBuild, "constraints", grippers.in(), handles.first,
               handles.second, shapes.second, envNames.in(), rules.in());
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      main->emitSendToBackground(item);
      main->logJobStarted(item->id(), "Build graph");
      lastId_ = item->id();
      tw_->deleteLater();
      tw_->close();
    }

    void HppManipulationWidgetsPlugin::autoBuildGraph()
    {
      tw_ = new QTabWidget;
      QListWidget* lw;
      QListWidget* grippers;
      QListWidget* handles;
      QPushButton* button = new QPushButton("Confirm", tw_);

      connect(button, SIGNAL(clicked()), SLOT(buildGraph()));
      hpp::Names_t_var n;
      n = hpp_->problem()->getAvailable("Gripper");
      QStringList l;
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      lw = new QListWidget(tw_);
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(lw, "Grippers");
      grippers = lw;

      n = hpp_->problem()->getAvailable("Handle");
      l.clear();
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      QWidget* widget = new QWidget(tw_);
      QVBoxLayout* box = new QVBoxLayout(widget);
      box->addWidget(new QLabel("The objects not selected will be locked in their current position."));
      lw = new QListWidget(tw_);
      box->addWidget(lw);
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(widget, "Handles");
      handles = lw;

      n = hpp_->problem()->getRobotContactNames();
      l.clear();
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      lw = new QListWidget(tw_);
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(lw, "Robot Contacts");

      n = hpp_->problem()->getEnvironmentContactNames();
      l.clear();
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      lw = new QListWidget(tw_);
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(lw, "Environments Contacts");

      LinkWidget* lWidget = new LinkWidget(grippers, handles, tw_);
      tw_->addTab(lWidget, "Rules");

      tw_->setCornerWidget(button, Qt::BottomRightCorner);
      tw_->show();
    }

    void HppManipulationWidgetsPlugin::handleWorkerDone(int id)
    {
      if (id == lastId_) {
          gepetto::gui::MainWindow::instance()->logJobDone(id, "Graph is build");
      }
    }

    void HppManipulationWidgetsPlugin::loadConstraintWidget()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      QDockWidget* dock = new QDockWidget ("&Constraint creator", main);
      constraintWidget_ = new ManipulationConstraintWidget (this, dock);
      dock->setWidget(constraintWidget_);
      main->insertDockWidget (dock, Qt::RightDockWidgetArea, Qt::Vertical);
      dock->toggleViewAction()->setShortcut(gepetto::gui::DockKeyShortcutBase + Qt::Key_V);
      dockWidgets_.append(dock);
      constraintWidget_->addConstraint(new PositionConstraint(this));
      constraintWidget_->addConstraint(new OrientationConstraint(this));
      constraintWidget_->addConstraint(new TransformConstraint(this));
      constraintWidget_->addConstraint(new ManipulationLockedJoint(this));
    }

    Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
  } // namespace gui
} // namespace hpp
