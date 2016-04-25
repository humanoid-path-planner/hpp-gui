#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "positionconstraintwidget.hh"

namespace hpp {
  namespace gui {
    PositionConstraintWidget::PositionConstraintWidget(QString const& firstJoint,
                                                       QString const& secondJoint,
                                                       QWidget* parent)
      : QDialog(parent)
    {
      QVBoxLayout* layout = new QVBoxLayout;

      firstX = new QDoubleSpinBox(this);
      firstY = new QDoubleSpinBox(this);
      firstZ = new QDoubleSpinBox(this);

      secondX = new QDoubleSpinBox(this);
      secondY = new QDoubleSpinBox(this);
      secondZ = new QDoubleSpinBox(this);

      layout->addWidget(new QLabel(firstJoint + " x", this));
      layout->addWidget(firstX);
      layout->addWidget(new QLabel(firstJoint + " y", this));
      layout->addWidget(firstY);
      layout->addWidget(new QLabel(firstJoint + " z", this));
      layout->addWidget(firstZ);
      layout->addWidget(new QLabel(firstJoint + " x", this));
      layout->addWidget(secondX);
      layout->addWidget(new QLabel(secondJoint + " y", this));
      layout->addWidget(secondY);
      layout->addWidget(new QLabel(secondJoint + " z", this));
      layout->addWidget(secondZ);

      QPushButton* button = new QPushButton(this);
      button->setText("Confirm");
      connect(button, SIGNAL(clicked()), SLOT(confirm()));
      layout->addWidget(button);
      this->setLayout(layout);
    }

    void PositionConstraintWidget::confirm()
    {
      QVector<double> vec;

      vec.push_front(firstX->value());
      vec.push_front(firstY->value());
      vec.push_front(firstZ->value());
      vec.push_front(secondX->value());
      vec.push_front(secondY->value());
      vec.push_front(secondZ->value());
      emit values(vec);
      this->done(0);
    }
  }
}
