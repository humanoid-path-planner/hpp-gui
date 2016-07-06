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
			  QListWidget* grippersList, QListWidget* handlesList,
                          QWidget *parent = 0);
      ~LinkWidget();

      Rules_var getRules();

    public slots:
      void gripperChanged(const QItemSelection& selected, const QItemSelection& deselected);
      void handleChanged(const QItemSelection& selected, const QItemSelection& deselected);

    private slots:
      void createRule();
    private:
      Ui::LinkWidget *ui_;
      std::vector<hpp::corbaserver::manipulation::Rule> rules_;
      QListWidget* grippers_;
      QListWidget* handles_;
    };
  }
}

#endif // LINKWIDGET_HH
