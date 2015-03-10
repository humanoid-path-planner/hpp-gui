#include <QtGui/QApplication>
#include "mainwindow.h"
#include "hpp-qt/corbaserver.hh"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  
  OmniORBThread omniThread (0, NULL);
  omniThread.start();

  return a.exec();
}
