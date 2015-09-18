#include "hpp/gui/ledindicator.h"

#include <QPainter>
#include <QPalette>
#include <QColor>
#include <QMouseEvent>

LedIndicator::LedIndicator(QWidget *parent):
  QWidget(parent),
  width(28), height(12) {
  setFixedSize(width, height);
  lit = false;
}

void LedIndicator::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.fillRect(0,0,width,height,lit?Qt::green:Qt::red);
}

void LedIndicator::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
      emit mouseClickEvent ();
    }
}

void LedIndicator::switchLed() {
  lit = ! lit;
  repaint();
  emit switched(lit);
}

void LedIndicator::switchLed(bool on) {
  if (lit == on) return;
  switchLed();
}
