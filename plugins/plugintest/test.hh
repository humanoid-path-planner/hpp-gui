//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#include <QObject>
#include <QWidget>

#include <iostream>

#include <gepetto/gui/plugin-interface.hh>

namespace hpp {
  namespace gui {
    class TestPlugin : public QObject, public PluginInterface {
      Q_OBJECT
        Q_INTERFACES (PluginInterface)

      public:
        void init();
    };
  } // namespace gui
} // namespace hpp
