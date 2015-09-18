#ifndef LEDINDICATOR_H
#define LEDINDICATOR_H

#include <QWidget>

class LedIndicator : public QWidget
{
  Q_OBJECT

public:
  LedIndicator (QWidget *parent = 0);

signals:
  void switched (bool on);
  void mouseClickEvent ();

public slots:
  void switchLed();
  void switchLed(bool on);

protected:
  void paintEvent(QPaintEvent *);
  void mouseReleaseEvent(QMouseEvent* event);

private:
  bool lit;
  const int width, height;
};

#endif // LEDINDICATOR_H
