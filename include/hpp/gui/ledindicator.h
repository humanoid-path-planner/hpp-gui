#ifndef HPP_GUI_LEDINDICATOR_HH
#define HPP_GUI_LEDINDICATOR_HH

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

#endif // HPP_GUI_LEDINDICATOR_HH
