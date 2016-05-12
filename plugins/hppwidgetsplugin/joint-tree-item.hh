#ifndef HPP_GUI_JOINTTREEITEM_HH
#define HPP_GUI_JOINTTREEITEM_HH

#include <QStandardItem>
#include <QItemDelegate>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QPushButton>

#include <hpp/corbaserver/robot.hh>
#include <gepetto/gui/fwd.hh>
#include <gepetto/viewer/node.h>

#include "hppwidgetsplugin/hppwidgetsplugin.hh"

namespace hpp {
  namespace gui {
    class IntegratorWheel : public QSlider
    {
      Q_OBJECT

      public:
        IntegratorWheel (Qt::Orientation o, HppWidgetsPlugin* plugin, QWidget *parent,
            gepetto::gui::MainWindow *main, std::string jointName,
            int nbDof, int index);

      protected:
        void timerEvent(QTimerEvent *);

        protected slots:
          void reset ();
        void updateIntegrator (int value);

      private:
        int rate_; // in millisecond
        int timerId_;
        gepetto::gui::MainWindow* main_;
        HppWidgetsPlugin* plugin_;

        std::string jointName_;
        const int bound_;
        const double maxVelocity_;
        double currentValue_;
        hpp::floatSeq_var dq_;
        int nbDof_, index_;
    };

    class SliderBoundedJoint : public QSlider
    {
      Q_OBJECT

      public:
        SliderBoundedJoint (Qt::Orientation orientation, HppWidgetsPlugin* plugin, QWidget* parent,
            gepetto::gui::MainWindow *main, std::string jointName,
            hpp::floatSeq* q, int index, double min, double max);

        double getValue ();

        private slots:
          void updateConfig (int value);

      private:
        gepetto::gui::MainWindow* main_;
        HppWidgetsPlugin* plugin_;
        std::string jointName_;
        hpp::floatSeq_var q_;
        int index_;
        double m_, M_;
    };

    class JointTreeItem : public QStandardItem
    {
      public:
        static const int IndexRole     ;
        static const int NumberDofRole ;
        static const int LowerBoundRole;
        static const int UpperBoundRole;
        static const int TypeRole      ;

        enum ItemType {
          SkipType = 0,
          IntegratorType,
          UnboundedValueType,
          BoundedValueType,
          BoundType
        };

        JointTreeItem (const char* name, const hpp::floatSeq& q,
            const hpp::corbaserver::jointBoundSeq& b, const unsigned int nbDof, graphics::NodePtr_t node);

        virtual QStandardItem* clone () const;

        virtual int type() {
          return QStandardItem::UserType+3;
        }

        const std::string& name () const {
          return name_;
        }

        hpp::floatSeq config () const;

        hpp::corbaserver::jointBoundSeq bounds () const;

        void updateBounds (const hpp::corbaserver::jointBoundSeq &b);

        void updateConfig (const hpp::floatSeq &c);

        void updateTypeRole ();

      private:
        typedef QList<QStandardItem*> StandardItemList;

        std::string name_;
        graphics::NodePtr_t node_;
        QVector<StandardItemList> value_;
    };

    class JointItemDelegate : public QItemDelegate
    {
      Q_OBJECT

      public:
        JointItemDelegate (QPushButton* forceVelocity, HppWidgetsPlugin* plugin, gepetto::gui::MainWindow* parent);

        void updateTypeRole (JointTreeItem::ItemType& type) const;

        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

      private:
        gepetto::gui::MainWindow* main_;
        HppWidgetsPlugin* plugin_;
        QPushButton* forceIntegrator_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_JOINTTREEITEM_HH
