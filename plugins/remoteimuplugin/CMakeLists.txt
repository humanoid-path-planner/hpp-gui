#
# Copyright (c) 2015, 2020 CNRS Authors: Joseph Mirabel, Guilhem Saurel
#
# This file is part of hpp-gui hpp-gui is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# hpp-gui is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU General Lesser Public License for more
# details.  You should have received a copy of the GNU Lesser General Public
# License along with hpp-gui  If not, see <http://www.gnu.org/licenses/>.

if(USE_QT4)
  set(QT "QT4")
else(USE_QT4)
  set(QT "")
endif(USE_QT4)

gepetto_gui_plugin(
  remoteimuplugin
  ${QT}
  HEADERS
  remoteimuplugin.hh
  SOURCES
  remoteimuplugin.cc
  LINK_DEPENDENCIES
  "hppwidgetsplugin"
  "hpp-corbaserver::hpp-corbaserver"
  PKG_CONFIG_DEPENDENCIES
  "remoteimu")
