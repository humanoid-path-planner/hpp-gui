#ifndef HPP_GUI_CONFIGURATIONLISTWIDGET_HH
#define HPP_GUI_CONFIGURATIONLISTWIDGET_HH

#include <QWidget>

#include "gepetto/gui/fwd.hh"
#include "gepetto/gui/mainwindow.hh"

#include "hpp/corbaserver/common.hh"

#include <hppwidgetsplugin/hppwidgetsplugin.hh>
#include "configurationlist.hh"

Q_DECLARE_METATYPE (hpp::floatSeq*)

namespace Ui {
  class ConfigurationListWidget;
}

namespace hpp {
  namespace gui {
    class ConfigurationListWidget : public QWidget
    {
      Q_OBJECT

      public:
        static const int ConfigRole;
        inline QListWidget* list ();

        void setInitConfig(hpp::floatSeq* config);

        ConfigurationListWidget(HppWidgetsPlugin* plugin, QWidget* parent = 0);

        virtual ~ConfigurationListWidget();

        public slots:
          void onSaveClicked ();
          void onDoubleClick(const QModelIndex& pos);
        void updateCurrentConfig (QListWidgetItem* current,QListWidgetItem* previous);

      private slots:
        void resetGoalConfigs (bool doEmpty = true);
        void setConfigs();


      private:
        inline QLineEdit* name ();
        void renameConfig(QListWidgetItem* item);

        HppWidgetsPlugin* plugin_;
        ::Ui::ConfigurationListWidget* ui_;
        QListWidget* previous_;

        gepetto::gui::MainWindow* main_;
        QString basename_;
        int count_;
    };

    class DropInitial : public QLabel
    {
    public:
      DropInitial(QWidget* parent);
      ~DropInitial();
      hpp::floatSeq* getConfig() const;

    protected:
      virtual void dragEnterEvent(QDragEnterEvent* event);
      virtual void dragMoveEvent(QDragMoveEvent* event);
      virtual void dropEvent(QDropEvent *event);
      virtual void mousePressEvent(QMouseEvent *event);
      virtual void mouseReleaseEvent(QMouseEvent *event);

      virtual void timerEvent(QTimerEvent *event);

    private:
      QListWidget* list_;
      hpp::floatSeq* fs_;

      QBasicTimer* timer_;
      bool alreadyReleased_;
    };

    QDataStream& operator>>(QDataStream& os, hpp::floatSeq*& tab);
    QDataStream& operator<<(QDataStream& os, hpp::floatSeq*& tab);
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_CONFIGURATIONLISTWIDGET_HH
