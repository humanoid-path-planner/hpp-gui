#ifndef HPP_GUI_LINKWIDGET_HH
#define LINKWIDGET_HH

#include <QWidget>

#include <hpp/corbaserver/manipulation/client.hh>

#include "hppmanipulationwidgetsplugin.hh"

namespace Ui {
class LinkWidget;
}

namespace hpp {
  namespace gui {
  using hpp::corbaserver::manipulation::Rules;
  using hpp::corbaserver::manipulation::Rules_var;

    class LinkWidget : public QWidget
    {
      Q_OBJECT

    public:
      explicit LinkWidget(HppManipulationWidgetsPlugin* plugins,
                          QWidget *parent = 0);
      ~LinkWidget();

      Rules_var getRules();

    private slots:
      void createRule();
    private:
      Ui::LinkWidget *ui_;
      std::vector<hpp::corbaserver::manipulation::Rule> rules_;
    };
  }
}

#endif // LINKWIDGET_HH
