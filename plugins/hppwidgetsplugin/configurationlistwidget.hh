//
// Copyright (c) CNRS
// Author: Joseph Mirabel and Heidy Dallard
//

#ifndef HPP_GUI_CONFIGURATIONLISTWIDGET_HH
#define HPP_GUI_CONFIGURATIONLISTWIDGET_HH

#include <QWidget>

#include "gepetto/gui/fwd.hh"
#include "gepetto/gui/mainwindow.hh"

#include "hpp/common-idl.hh"

#include "configurationlist.hh"

Q_DECLARE_METATYPE (hpp::floatSeq*)

namespace Ui {
  class ConfigurationListWidget;
}

namespace hpp {
  namespace gui {
    class HppWidgetsPlugin;

    /// Widget to define initial and goal configurations of the problem.
    class ConfigurationListWidget : public QWidget
    {
      Q_OBJECT

      public:
        static const int ConfigRole;
        /// Get the list of configurations.
        QListWidget* list ();

        /// Set the initial configuration in the problem.
        void setInitConfig(hpp::floatSeq& config);

        ConfigurationListWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);

        virtual ~ConfigurationListWidget();

        static QListWidgetItem* makeItem (QString text, const hpp::floatSeq& q);

        static hpp::floatSeq& getConfig (QListWidgetItem* item);

        void reciveConfig (QString name, const hpp::floatSeq& config);

      public slots:
        /// Save the current configuration of the robot.
        void onSaveClicked ();

        /// Change the configuration displayed in the viewer.
        /// \param current new configuration
        /// \param previous previous configuration
        void updateCurrentConfig (QListWidgetItem* current,QListWidgetItem* previous);

        void fetchInitAndGoalConfigs ();

      private slots:
        /// Reset the goals configuration in the problem.
        /// \param doEmpty if true empty the list else doesn't
        void resetGoalConfigs (bool doEmpty = true);

        /// Set the goals configuration in the problem.
        void setConfigs();

      private:
        /// Get the LineEdit that hold the name of the configuration.
        inline QLineEdit* name ();

        /// Rename the configuration pointed by item.
        /// \param item list item that holds the configuration to rename
        void renameConfig(QListWidgetItem* item);

        HppWidgetsPlugin* plugin_;
        ::Ui::ConfigurationListWidget* ui_;
        QListWidget* previous_;

        gepetto::gui::MainWindow* main_;
        QString basename_;
        int count_;
    };

    QDataStream& operator>>(QDataStream& os, hpp::floatSeq& tab);
    QDataStream& operator<<(QDataStream& os, const hpp::floatSeq& tab);
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_CONFIGURATIONLISTWIDGET_HH
