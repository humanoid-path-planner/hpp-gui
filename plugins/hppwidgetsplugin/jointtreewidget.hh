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

        /// Set the dock widget.
        /// \param dock new dock widget
        void dockWidget (QDockWidget* dock);

        /// Get the currently selected joint.
        std::string selectedJoint () const;

signals:

        public slots:
          /// Display the context menu for a givent joint.
          /// \param pos poistion of the joint in the tree
          void customContextMenu (const QPoint& pos);

        /// select the joint in the tree.
        /// \param jointName name of the joint selected
        void selectJoint (const std::string& jointName);

        /// Get the currently selected joint name.
        QString getSelectedJoint () const;

        /// Open a dialog to set a joint bounds.
        /// \param jointName name of the joint
        void openJointBoundDialog (const std::string jointName);

        /// Open a dialog to move a joint.
        /// \param jointName name of the joint
        void openJointMoveDialog(const std::string jointName);

        /// Call the corba function to move the joint in hpp.
        /// \param transform transform to apply
        /// \param jointName name of the joint
        void moveJoint(hpp::Transform__slice* transform, std::string const& jointName);

        /// Reload the joint in the tree.
        void reload ();

        private slots:
          void resize (const QModelIndex index);

      private:
        /// Reset the tree.
        void reset ();

        JointTreeItem* buildJointTreeItem (const char* name);

        HppWidgetsPlugin* plugin_;
        ::Ui::JointTreeWidget* ui_;

        QStandardItemModel* model_;
        QDockWidget* dock_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_JOINTTREEWIDGET_HH
