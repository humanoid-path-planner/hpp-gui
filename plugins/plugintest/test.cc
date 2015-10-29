#include <test.hh>
#include <QtPlugin>

namespace hpp {
  namespace gui {
    void TestPlugin::init() {
      qDebug() << "test";
      std::cout << "test" << std::endl;
    }

    Q_EXPORT_PLUGIN2 (testplugin, TestPlugin)
  } // namespace gui
} // namespace hpp
