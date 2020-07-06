//
// Copyright (c) CNRS
// Authors: Yann de Mont-Marin
//
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QString>

#include <boost/lexical_cast.hpp> 
#include <boost/algorithm/string.hpp>

#include "hppwidgetsplugin/demoloadersubwidget.hh"
#include <hppwidgetsplugin/hppwidgetsplugin.hh>
#include <gepetto/gui/mainwindow.hh>
#include "hpp/corbaserver/client.hh"


namespace hpp {
  namespace gui {

    using gepetto::gui::MainWindow;
    using CORBA::ULong;

    DemoLoaderSubWidget::DemoLoaderSubWidget(HppWidgetsPlugin *plugin):
      QObject (plugin),
      plugin_ (plugin)
    {
    }



    void DemoLoaderSubWidget::init(){
      MainWindow* main = MainWindow::instance();
      QToolBar* toolBar = MainWindow::instance()->addToolBar("demo loading toolbar");
      toolBar->setObjectName ("demoloading.toolbar");

      QAction* load = new QAction ("Load demo", toolBar);
      toolBar->addAction (load);
      connect (load, SIGNAL(triggered()), SLOT (loadDemo()));
      main->registerSlot("loadDemo", this);

      QAction* save = new QAction ("Save demo", toolBar);
      toolBar->addAction (save);
      connect (save, SIGNAL(triggered()), SLOT (saveDemo()));
      main->registerSlot("saveDemo", this);
    }

    std::string DemoLoaderSubWidget::robotName(){
      CORBA::String_var robotName = plugin_->client ()->robot()->getRobotName();
      return (std::string) robotName.in();
    }

    void DemoLoaderSubWidget::loadDemo(){
      MainWindow* main = MainWindow::instance();

      QString filename = QFileDialog::getOpenFileName (NULL, "Select a demo file");
      if (filename.isNull()) return;
      std::string filename_ (filename.toStdString());

      TiXmlDocument doc(filename_);
      if(!doc.LoadFile()){
        const char* msg = "Unable to load demo file";
        if (main != NULL)
          main->logError(msg);
        return;
      }

      TiXmlHandle hdl(&doc);
      TiXmlElement* problem_element = hdl.FirstChildElement().Element();

      // Check that robot match
      std::string root_type = problem_element->ValueStr ();
      std::string robot_name = problem_element->Attribute("robotname");
      if (
        root_type.compare("problem")!=0 || robot_name.compare(robotName())!=0
      ){
        const char* msg = "The robot and the demo file does not match";
        if (main != NULL)
          main->logError(msg);
        return;
      }

      // Loop on all elements
      TiXmlElement *elem = hdl.FirstChildElement().FirstChildElement().Element();
      while (elem){
        // Get name
        std::string name = elem->Attribute("name");

        // Parse floatSeq
        std::vector<std::string> parsing;
        std::string txt = elem->GetText();
        boost::split(parsing, txt, boost::is_any_of(" "));

        hpp::floatSeq* fS = new hpp::floatSeq();
        fS->length((ULong) parsing.size());
        for(size_t i=0; i<parsing.size(); i++){
          (*fS)[(ULong) i] = boost::lexical_cast<double> (parsing[i]);
        }

        // Get element type
        std::string type = elem->ValueStr ();

        if (type.compare("joint") == 0){
          loadBound(name, *fS);
        }
        else if (type.compare("config") == 0){
          loadConfig(name, *fS);
        }
        elem = elem->NextSiblingElement(); // iteration 
      }
    }

    void DemoLoaderSubWidget::saveDemo(){
      QString filename = QFileDialog::getSaveFileName(NULL, tr("Select a destination"));
      if (filename.isNull()) return;
      std::string filename_ (filename.toStdString());

      TiXmlDocument doc;
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
      doc.LinkEndChild( decl );

      TiXmlElement * problem_element = new TiXmlElement("problem");
      problem_element->SetAttribute("robotname", robotName());
      doc.LinkEndChild(problem_element);

      writeBounds(problem_element);

      writeConfigs(problem_element);

      doc.SaveFile(filename_.c_str());
    }

    void DemoLoaderSubWidget::loadBound(const std::string & name, const hpp::floatSeq & fS){
      CORBA::Long nbDof = plugin_->client()->robot ()->getJointNumberDof(name.c_str());
      if (nbDof > 0) {
        hpp::floatSeq_var bounds =
          plugin_->client()->robot()->getJointBounds(name.c_str());
        hpp::floatSeq& b = bounds.inout();
        for (size_t i=0; i < b.length (); ++i){
          b[(ULong) i] = fS[(ULong) i];
        }
      plugin_->client()->robot()->setJointBounds(name.c_str(), b);
      }
    }

    void DemoLoaderSubWidget::loadConfig(const std::string & name, const hpp::floatSeq & fS){
      clWidget()->reciveConfig(QString::fromUtf8(name.c_str()), fS);
    }

    void DemoLoaderSubWidget::writeBounds(TiXmlElement* parent){
      hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames ();
      for (size_t i = 0; i < joints->length (); ++i) {
        const char* jointName = joints[(ULong) i];
        CORBA::Long nbDof = plugin_->client()->robot ()->getJointNumberDof(jointName);
        if (nbDof > 0) {
          hpp::floatSeq_var bounds =
            plugin_->client()->robot()->getJointBounds(jointName);
        
          const hpp::floatSeq& b = bounds.in();
          writeElement(parent, "joint",
                       (std::string) jointName,
                       b);
        }
      }
    }

    void DemoLoaderSubWidget::writeConfigs(TiXmlElement* parent){
      for (int i = 0; i < clWidget()->list()->count(); ++i){
        QListWidgetItem* item = clWidget()->list()->item(i);
        writeElement(parent, "config",
                     item->text().toStdString(),
                     clWidget()->getConfig(item));
      }
    }

    void DemoLoaderSubWidget::writeElement(TiXmlElement* parent,
                                           std::string type,
                                           std::string name,
                                           const hpp::floatSeq & fS){
      // Convert hpp floafSeq
      std::string textual_seq("");
      for (size_t i=0; i< fS.length(); ++i){
        textual_seq += boost::lexical_cast<std::string>(fS[(ULong) i]);
        if (i != fS.length() -1){
          textual_seq += " ";
        }
      }
      // Create node
      TiXmlElement * element = new TiXmlElement(type);
      element->SetAttribute("name", name );
      TiXmlText * text = new TiXmlText(textual_seq);
      element->LinkEndChild(text);
      parent->LinkEndChild(element);
    }

    ConfigurationListWidget* DemoLoaderSubWidget::clWidget() const {
      return plugin_->configurationListWidget();
    }
  } // namespace gui
} // namespace hpp
