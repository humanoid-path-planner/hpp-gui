//
// Copyright (c) CNRS
// Authors: Yann de Mont-Marin
//

#ifndef HPP_GUI_DEMOLOADERSUBWIDGET_HH
#define HPP_GUI_DEMOLOADERSUBWIDGET_HH

# include <string>
# include <tinyxml.h>

#include <hppwidgetsplugin/hppwidgetsplugin.hh>
#include <hppwidgetsplugin/configurationlistwidget.hh>

namespace hpp {
  namespace gui {
    class DemoLoaderSubWidget : public QObject
    {
      Q_OBJECT

      public:
        explicit DemoLoaderSubWidget(HppWidgetsPlugin *plugin);
        void init();

      public slots:
        void loadDemo();
        void saveDemo();
        std::string robotName();

      protected:
        void writeBounds(TiXmlElement* parent);
        void writeConfigs(TiXmlElement* parent);
        void writeElement(TiXmlElement* parent, std::string type, std::string name, const hpp::floatSeq & fS);
        void loadConfig(const std::string & name, const hpp::floatSeq & fS);
        void loadBound(const std::string & name, const hpp::floatSeq & fS);

      private:
        HppWidgetsPlugin* plugin_;
        ConfigurationListWidget* clWidget() const;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_DEMOLOADERSUBWIDGET_HH
