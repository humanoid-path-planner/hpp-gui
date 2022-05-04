// Copyright (c) 2019 CNRS
// Authors: Joseph Mirabel
//
//
// This file is part of hpp-gui
// hpp-gui is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-gui is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-gui  If not, see
// <http://www.gnu.org/licenses/>.

#include <QObject>
#include <gepetto/gui/plugin-interface.hh>

namespace hpp {
namespace gui {
class HppFclPlugin : public QObject, public gepetto::gui::PluginInterface {
  Q_OBJECT
  Q_INTERFACES(gepetto::gui::PluginInterface)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  Q_PLUGIN_METADATA(IID "hpp-gui.hppwidgetsplugin")
#endif

 public:
  QString name() const { return QString("HppFclPlugin"); }

 public slots:
  void addBV(QString name, QString filename, int splitMethod) const;

 protected:
  void init();

 protected slots:
  void openDialog() const;
};
}  // namespace gui
}  // namespace hpp
