//
// Copyright (c) CNRS
// Authors: Joseph Mirabel
//

#ifndef HPP_GUI_JOINTTREEITEM_HH
#define HPP_GUI_JOINTTREEITEM_HH

#include <QItemDelegate>
#include <QSlider>
#include <QStandardItem>

#include <hpp/corbaserver/robot.hh>
#include <gepetto/gui/fwd.hh>
#include <gepetto/viewer/node.h>

#include "hppwidgetsplugin/hppwidgetsplugin.hh"

class QDoubleSpinBox;
class QMenu;
class QPushButton;

namespace hpp {
  namespace gui {
    class JointTreeItem : public QStandardItem
    {
      public:
        typedef CORBA::ULong ULong;
        typedef gepetto::viewer::NodePtr_t NodePtr_t;
        typedef std::vector<NodePtr_t> NodesPtr_t;

        static const int IndexRole     ;
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

        JointTreeItem (const char* name,
            const ULong& idxQ,
            const ULong& idxV,
            const hpp::floatSeq& q,
            const hpp::floatSeq& b,
            const ULong& nbDof,
            const NodesPtr_t& node);

        ~JointTreeItem ();

        virtual QStandardItem* clone () const;

        virtual int type() {
          return QStandardItem::UserType+3;
        }

        const std::string& name () const {
          return name_;
        }

        hpp::floatSeq config () const;

        hpp::floatSeq bounds () const;

        ULong rankInConfig () const { return idxQ_; }

        ULong rankInVelocity () const { return idxV_; }

        ULong configSize () const { return nq_; }

        ULong numberDof () const { return nv_; }

        void updateBounds (const hpp::floatSeq &b);

        void updateConfig (const hpp::floatSeq &c);

        void updateFromRobotConfig (const hpp::floatSeq &c);

        void updateTypeRole ();

        void setupActions(HppWidgetsPlugin* plugin);

        const QList<QAction*>& actions () const;

      private:
        typedef QList<QStandardItem*> StandardItemList;

        std::string name_;
        ULong idxQ_, idxV_, nq_, nv_;
        NodesPtr_t nodes_;
        QVector<StandardItemList> value_;
        QList<QAction*> actions_;
    };

    class IntegratorWheel : public QSlider
    {
      Q_OBJECT

      public:
        IntegratorWheel (Qt::Orientation o, HppWidgetsPlugin* plugin, QWidget *parent,
            gepetto::gui::MainWindow *main, JointTreeItem const* item, int index);

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
        JointTreeItem const* item_;

        const int bound_;
        const double maxVelocity_;
        hpp::floatSeq q_, dq_;
        int index_;
    };

    class SliderBoundedJoint : public QSlider
    {
      Q_OBJECT

      public:
        SliderBoundedJoint (Qt::Orientation orientation, HppWidgetsPlugin* plugin, QWidget* parent,
            gepetto::gui::MainWindow *main, JointTreeItem const* item, int index);

        double getValue ();

        private slots:
          void updateConfig (int value);

      private:
        gepetto::gui::MainWindow* main_;
        HppWidgetsPlugin* plugin_;
        JointTreeItem const* item_;
        double value_;
        int index_;
        double m_, M_;
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
