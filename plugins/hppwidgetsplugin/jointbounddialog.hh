//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#ifndef HPP_GUI_JOINTBOUNDDIALOG_HH
#define HPP_GUI_JOINTBOUNDDIALOG_HH

#include <hpp/corbaserver/robot.hh>
#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>

namespace hpp {
  namespace gui {
    class JointBoundDialog : public QDialog
    {
      Q_OBJECT

      public:
        explicit JointBoundDialog(QString name, std::size_t nbDof, QWidget *parent = 0);

        void setBounds (const hpp::floatSeq& bounds);

        void getBounds (hpp::floatSeq& bounds) const;

        ~JointBoundDialog();

      private:
        struct Line {
          QLabel* label;
          QDoubleSpinBox *min, *max;
          Line (const QString& name, QWidget* parent);
          void addToLayout (QLayout* l);
        };

        QList <Line> lines_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_JOINTBOUNDDIALOG_HH
