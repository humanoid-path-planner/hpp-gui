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
        typedef graphics::NodePtr_t NodePtr_t;
        typedef std::vector<NodePtr_t> NodesPtr_t;

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

        JointTreeItem (const char* name,
            const std::size_t& idxQ,
            const hpp::floatSeq& q,
            const hpp::floatSeq& b,
            const unsigned int nbDof,
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

        void updateBounds (const hpp::floatSeq &b);

        void updateConfig (const hpp::floatSeq &c);

        void updateFromRobotConfig (const hpp::floatSeq &c);

        void updateTypeRole ();

        void setupActions(HppWidgetsPlugin* plugin);

        const QList<QAction*>& actions () const;

      private:
        typedef QList<QStandardItem*> StandardItemList;

        std::string name_;
        std::size_t idxQ_;
        NodesPtr_t nodes_;
        QVector<StandardItemList> value_;
        QList<QAction*> actions_;
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
