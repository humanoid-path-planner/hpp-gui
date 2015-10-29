#ifndef HPP_GUI_JOINTTREEWIDGET_HH
#define HPP_GUI_JOINTTREEWIDGET_HH

#include <QWidget>

#include <hppwidgetsplugin/hppwidgetsplugin.hh>

namespace Ui {
  class JointTreeWidget;
}

namespace hpp {
  namespace gui {
    class JointTreeWidget : public QWidget
    {
      Q_OBJECT

      public:
        explicit JointTreeWidget(HppWidgetsPlugin *plugin, QWidget *parent = 0);

        virtual ~JointTreeWidget ();

        void dockWidget (QDockWidget* dock);

        std::string selectedJoint ();

signals:

        public slots:
          void customContextMenu (const QPoint& pos);
        void addJointToTree (const std::string name, JointTreeItem *parent);
        void selectJoint (const std::string& jointName);
        void openJointBoundDialog (const std::string jointName);

        void reload ();

        private slots:
          void resize (const QModelIndex index);

      private:
        void reset ();

        HppWidgetsPlugin* plugin_;
        ::Ui::JointTreeWidget* ui_;

        QStandardItemModel* model_;
        QDockWidget* dock_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_JOINTTREEWIDGET_HH
