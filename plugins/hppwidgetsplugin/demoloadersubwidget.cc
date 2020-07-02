//
// Copyright (c) CNRS
// Authors: Joseph Mirabel and Heidy Dallard
//

#include "hppwidgetsplugin/demoloadersubwidget.hh"
#include <hppwidgetsplugin/hppwidgetsplugin.hh>
#include <gepetto/gui/mainwindow.hh>
#include "hpp/corbaserver/client.hh"
#include <boost/algorithm/string.hpp>


namespace hpp {
  namespace gui {

    using gepetto::gui::MainWindow;
    using CORBA::ULong;

    void DemoLoaderSubWidget::DemoLoaderSubWidget(HppWidgetsPlugin *plugin,
                                                  ConfigurationListWidget *configWidget);
      QObject (plugin),
      plugin_ (plugin),
      configWidget_ (configWidget)
    {
    }

    void DemoLoaderSubWidget::init(){
      MainWindow* main = MainWindow::instance();
      QToolBar* toolBar = MainWindow::instance()->addToolBar("demo loading toolbar");
      toolBar->setObjectName ("demoloading.toolbar");

      QAction* load = new QAction ("Load demo", toolBar);
      toolBar->addAction (load);
      connect (reset, SIGNAL(triggered()), SLOT (loadDemo()));
      main->registerSlot("loadDemo", this);

      QAction* save = new QAction ("Save demo", toolBar);
      toolBar->addAction (save);
      connect (reset, SIGNAL(triggered()), SLOT (saveDemo()));
      main->registerSlot("saveDemo", this);
    }

    void DemoLoaderSubWidget::robotName(){
      return (std::string) CORBA::String_var robotName = plugin_->client ()->robot()->getRobotName();
    }

    void DemoLoaderSubWidget::loadDemo(){
      MainWindow* main = MainWindow::instance();

      QString filename = QFileDialog::getOpenFileName (this, "Select a demo file");
      if (file.isNull()) return;
      std::string filename_ (filename.toStdString());

      TiXmlDocument doc(filename_);
      if(!doc.LoadFile()){
        const char* msg = "Unable to load demo file";
        if (main != NULL)
          main->logError(msg);
        return;
      }

      TiXmlHandle hdl(&doc);
      TiXmlElement* problem_element = hdl.FirstChildElement();

      // Check that robot match
      if (
        elem->ValueStr().compare("problem")!=0 ||
        robotName().compare((std::string) problem_element->Attribute("name")) !=0
      ){
        const char* msg = "The robot and the demo file does not match";
        if (main != NULL)
          main->logError(msg);
        return;
      }

      // Loop on all elements
      TiXmlElement *elem = hproblem_element->FirstChildElement().Element();
      while (elem){
        // Get name
        std:string name = elem->Attribute("name");

        // Parse floatSeq
        std::vector<std::string> parsing;
        boost::split(parsing, elem->GetText(), [](char c){return c == ' ';});

        hpp::floatSeq* fS = new hpp::floatSeq();
        fS->length((ULong) parsing.size());
        for(int i=0; i<parsing.size(); i++){
          (*fS)[(ULong) i] = boost::lexical_cast<double> (results[i]);
        }

        // Get element type
        std::string type = elem->ValueStr ()

        if (type.compare("joint") = 0){
          loadBound(name, *fS)
        }
        else if (type.compare("joint") = 0){
          loadConfig(name, *fS)
        }
        elem = elem->NextSiblingElement(); // iteration 
      }
    }

    void DemoLoaderSubWidget::saveDemo(){
      MainWindow* main = MainWindow::instance();

      QString filename = QFileDialog::getSaveFileName(this, tr("Select a destination"));
      if (filename.isNull()) return;
      std::string filename_ (filename.toStdString());

      TiXmlDocument doc;
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
      doc.LinkEndChild( decl );

      TiXmlElement * problem_element = new TiXmlElement("problem");
      problem_element->SetAttribute("robotname", robotname());
      doc->LinkEndChild(problem_element);

      writeBounds(problem_element);

      writeConfigs(problem_element);

      doc.SaveFile(filename);
    }

    void DemoLoaderSubWidget::loadBound(std::string name,  hpp::floatSeq & fS){
      CORBA::Long nbDof = plugin_->client()->robot ()->getJointNumberDof(name);
      if (nbDof > 0) {
        hpp::floatSeq_var bounds =
          plugin_->client()->robot()->getJointBounds(name.c_str());
        hpp::floatSeq& b = bounds.inout();
        for (size_t i=0; i < bounds.length (); ++i){
          b[(ULong) i] = fS[(ULong) i];
        }
      plugin_->client()->robot()->setJointBounds(name.c_str(), bounds.in());
      }
    }

    void DemoLoaderSubWidget::loadConfig(std::string name, hpp::floatSeq & fS){
      configWidget_->reciveConfig(Qstring(name), fS);
    }

    void DemoLoaderSubWidget::writeBounds(TiXmlElement* parent){
      hpp::Names_t_var joints = plugin_->client()->robot()->getAllJointNames ();
      for (size_t i = 0; i < joints->length (); ++i) {
        const char* jointName = joints[(ULong) i];
        CORBA::Long nbDof = plugin_->client()->robot ()->getJointNumberDof(jointName);
        if (nbDof > 0) {
          hpp::floatSeq_var bounds =
            _plugin->client()->robot()->getJointBounds(jointName.c_str());
          writeElement(parent, "joint",
                       (std::string) jointName,
                       bounds.in())
        }
      }
    }

    void DemoLoaderSubWidget::writeConfigs(TiXmlElement* parent){
      for (int i = 0; i < configWidget_->list->count(); ++i){
        QListWidgetItem* item = configWidget_->list->item(i)


        const hpp::floatSeq& config ;
            writeElement(parent, "config",
                         item->text().toStdString(),
                         configWidget_->getConfig(item))

          // OUTPUT
          Qstring name = ;
      }

    };

    void DemoLoaderSubWidget::writeElement(TiXmlElement* parent,
                                           std::string type,
                                           std::string name,
                                           hpp::floatSeq & fS){
      // Convert hpp floafSeq
      std::string textual_seq("");
      for (size_t i=0; i< fS.length(); ++i){
        textual_seq += boost::lexical_cast<std::string>(fS[(ULong) i])
        if !(i< fS.length() -1){
          textual_seq += " "
        }
      }
      // Create node
      TiXmlElement * element = new TiXmlElement(type);
      element->SetAttribute("name", name );
      TiXmlText * text = new TiXmlText(textual_seq);
      element->LinkEndChild(text);
      parent->LinkEndChild(element);
    }

  } // namespace gui
} // namespace hpp
