//
// Copyright (c) CNRS
// Author: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_DEMOLOADERSUBWIDGET_HH
#define HPP_GUI_DEMOLOADERSUBWIDGET_HH

#include <hppwidgetsplugin/hppwidgetsplugin.hh>
#include <hppwidgetsplugin/configurationlistwidget.hh>

namespace hpp {
  namespace gui {
    class DemoLoaderSubWidget : public QObject
    {
      Q_OBJECT

      public:
        explicit DemoLoaderSubWidget(HppWidgetsPlugin *plugin,
                                     ConfigurationListWidget *configWidget);
        void init();

      public slots:
        void loadDemo();
        void saveDemo();
        std::string robotName();

      protected:
        void writeBounds(TiXmlElement* parent);
        void writeConfigs(TiXmlElement* parent);
        void writeElement(TiXmlElement* parent, std::string type, std::string name, hpp::floatSeq & fS);
        void loadConfig(std::string name, hpp::floatSeq & fS);
        void loadBound(std::string name, hpp::floatSeq & fS);

      private:
        HppWidgetsPlugin* plugin_;
        ConfigurationListWidget* configWidget_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_DEMOLOADERSUBWIDGET_HH
