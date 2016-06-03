#include "hppmanipulationwidgetsplugin/hppmanipulationwidgetsplugin.hh"

#include "hppmanipulationwidgetsplugin/roadmap.hh"
#include "gepetto/gui/mainwindow.hh"
#include "gepetto/gui/windows-manager.hh"

#include "hpp/manipulation/graph/helper.hh"

#include "hppwidgetsplugin/jointtreewidget.hh"
#include "linkwidget.hh"

using CORBA::ULong;

namespace hpp {
  namespace gui {
    HppManipulationWidgetsPlugin::HppManipulationWidgetsPlugin() :
      HppWidgetsPlugin (),
      hpp_ (NULL),
      toolBar_ (NULL),
      tw_ (NULL)
    {
      firstEnter_ = 0;
    }

    HppManipulationWidgetsPlugin::~HppManipulationWidgetsPlugin()
    {
      if (tw_) tw_->deleteLater();
    }

    void HppManipulationWidgetsPlugin::init()
    {
      HppWidgetsPlugin::init ();

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
    }

    QString HppManipulationWidgetsPlugin::name() const
    {
      return QString ("Widgets for hpp-manipulation-corba");
    }

    void HppManipulationWidgetsPlugin::loadRobotModel(gepetto::gui::DialogLoadRobot::RobotDefinition rd)
    {
      if (firstEnter_ == 0) {
      	hpp_->robot ()->create (gepetto::gui::Traits<QString>::to_corba("composite").in());
      	firstEnter_ = 1;
      }
      hpp_->robot ()->insertRobotModel (gepetto::gui::Traits<QString>::to_corba(rd.robotName_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.rootJointType_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.package_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.modelName_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.urdfSuf_).in(),
					gepetto::gui::Traits<QString>::to_corba(rd.srdfSuf_).in());
      updateRobotJoints (rd.robotName_);
      jointTreeWidget_->addJointToTree("base_joint", 0);
      applyCurrentConfiguration();
      gepetto::gui::MainWindow::instance()->requestRefresh();
      emit logSuccess ("Robot " + rd.name_ + " loaded");
    }

    void HppManipulationWidgetsPlugin::loadEnvironmentModel(gepetto::gui::DialogLoadEnvironment::EnvironmentDefinition ed)
    {
      if (firstEnter_ == 0) {
      	hpp_->robot ()->create (gepetto::gui::Traits<QString>::to_corba("composite").in());
      	firstEnter_ = 1;
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
      QByteArray iiop = getIIOPurl ().toAscii();
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
        std::string linkName (client()->robot()->getLinkName (jname));
        jointMap_[jname] = JointElement(jname, linkName, 0, true);
      }
    }

    Roadmap *HppManipulationWidgetsPlugin::createRoadmap(const std::string &jointName)
    {
      ManipulationRoadmap* r = new ManipulationRoadmap(this);
      r->initRoadmap(jointName);
      return r;
    }

    void HppManipulationWidgetsPlugin::drawRobotContacts()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getRobotContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getRobotContact (
            rcs[i], indexes.out(), points.out());
        CORBA::Long iPts = 0;
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Create group
          std::string target = createJointGroup(std::string(cjs[j]));

          /// Add the contacts
          std::stringstream ssname;
          ssname <<  target << "/contact_"
            << escapeJointName(std::string (rcs[i])) << "_" << j;
          std::string name = ssname.str ();
          gepetto::corbaserver::PositionSeq ps; ps.length (indexes[j] - iPts);
          for (CORBA::Long k = iPts; k < indexes[j]; ++k) {
            ps[k - iPts][0] = (float)points[k][0];
            ps[k - iPts][1] = (float)points[k][1];
            ps[k - iPts][2] = (float)points[k][2];
          }
          iPts = indexes[j];
          main->osg()->addCurve (name.c_str(), ps, color);
          main->osg()->setCurveMode (name.c_str(), GL_POLYGON);
        }
      }
      gepetto::gui::MainWindow::instance()->requestRefresh();
    }

    void HppManipulationWidgetsPlugin::drawEnvironmentContacts()
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance ();
      hpp::Names_t_var rcs = hpp_->problem()->getEnvironmentContactNames();
      hpp::floatSeqSeq_var points;
      hpp::intSeq_var indexes;
      const float color[] = {0, 1, 0, 1};
      for (CORBA::ULong i = 0; i < rcs->length(); ++i) {
        hpp::Names_t_var cjs = hpp_->problem()->getEnvironmentContact (
            rcs[i], indexes.out(), points.out());
        CORBA::Long iPts = 0;
        for (CORBA::ULong j = 0; j < cjs->length(); ++j) {
          /// Add the contacts
          std::stringstream ssname;
          ssname << "hpp-gui/contact_"
            << escapeJointName(std::string (rcs[i])) << "_" << j;
          std::string name = ssname.str ();
          gepetto::corbaserver::PositionSeq ps; ps.length (indexes[j] - iPts);
          for (CORBA::Long k = iPts; k < indexes[j]; ++k) {
            ps[k - iPts][0] = (float)points[k][0];
            ps[k - iPts][1] = (float)points[k][1];
            ps[k - iPts][2] = (float)points[k][2];
          }
          iPts = indexes[j];
          main->osg()->addCurve (name.c_str(), ps, color);
          main->osg()->setCurveMode (name.c_str(), GL_POLYGON);
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
        for (int i = 0; i < 7; ++i) t_gv[i] = t.in()[i];
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
        for (int i = 0; i < 7; ++i) t_gv[i] = t.in()[i];
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

    HppManipulationWidgetsPlugin::NamesPair
    HppManipulationWidgetsPlugin::buildNamess(const QList<QListWidgetItem *>& names)
    {
      std::map<std::string, std::list<std::string> > mapNames;

      for (int i = 0; i < names.count(); i++) {
        std::string name(names[i]->text().toStdString());
	size_t pos = name.find_first_of("/");
	std::string object = name.substr(0, pos);

	mapNames[object].push_back(name);
      }
      return convertMap(mapNames);
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

    void HppManipulationWidgetsPlugin::buildGraph()
    {
      QListWidget* l = dynamic_cast<QListWidget*>(tw_->widget(0));
      hpp::Names_t_var grippers = convertToNames(l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(1));
      HppManipulationWidgetsPlugin::NamesPair handles = buildNamess(l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(2));
      HppManipulationWidgetsPlugin::NamesPair shapes = buildNamess(l->selectedItems());
      l = dynamic_cast<QListWidget*>(tw_->widget(3));
      hpp::Names_t_var envNames = convertToNames(l->selectedItems());
      hpp::corbaserver::manipulation::Rules_var rules = dynamic_cast<LinkWidget *>(tw_->widget(4))->getRules();

      std::cout << "length = " << rules->length() << std::endl;
      for (unsigned i = 0; i < rules->length(); i++) {
        std::cout << rules[i].gripper << " - " << rules[i].handle
                  << " - " << rules[i].link << std::endl;
      }

      hpp_->graph ()->createGraph("constraints");
      hpp_->graph ()->autoBuild("constraints", grippers.in(), handles.first,
                handles.second, shapes.second, envNames.in(), rules.in());
      tw_->deleteLater();
      tw_->close();
    }

    void HppManipulationWidgetsPlugin::autoBuildGraph()
    {
      tw_ = new QTabWidget;
      QListWidget* lw;
      QPushButton* button = new QPushButton("Confirm", tw_);

      connect(button, SIGNAL(clicked()), SLOT(buildGraph()));
      hpp::Names_t_var n;
      n = hpp_->problem()->getAvailable("Gripper");
      QStringList l;
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      lw = new QListWidget(tw_);
      std::cout << lw << std::endl;
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(lw, "Grippers");

      n = hpp_->problem()->getAvailable("Handle");
      l.clear();
      for (unsigned i = 0; i < n->length(); i++) {
        l << QString(n[i].in());
      }
      lw = new QListWidget(tw_);
      lw->addItems(l);
      lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tw_->addTab(lw, "Handles");

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

      LinkWidget* lWidget = new LinkWidget(this, tw_);
      tw_->addTab(lWidget, "Rules");

      tw_->setCornerWidget(button, Qt::BottomRightCorner);
      tw_->show();
    }

    Q_EXPORT_PLUGIN2 (hppmanipulationwidgetsplugin, HppManipulationWidgetsPlugin)
  } // namespace gui
} // namespace hpp
