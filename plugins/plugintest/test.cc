#include <test.hh>
#include <QtPlugin>

void TestPlugin::init() {
  qDebug() << "test";
  std::cout << "test" << std::endl;
}

Q_EXPORT_PLUGIN2 (testplugin, TestPlugin)
