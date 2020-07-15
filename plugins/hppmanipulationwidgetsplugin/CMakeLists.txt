#
# Copyright (c) 2015, 2020 CNRS
# Authors: Joseph Mirabel, Guilhem Saurel
#
#
# This file is part of hpp-gui
# hpp-gui is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# hpp-gui is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# hpp-gui  If not, see
# <http://www.gnu.org/licenses/>.

IF(USE_QT4)
  SET(QT "QT4")
ELSE(USE_QT4)
  SET(QT "")
ENDIF(USE_QT4)

GEPETTO_GUI_PLUGIN(hppmanipulationwidgetsplugin
  ${QT}

  HEADERS
  hppmanipulationwidgetsplugin.hh
  linkwidget.hh

  HEADERS_NO_MOC
  roadmap.hh

  FORMS
  linkwidget.ui

  SOURCES
  hppmanipulationwidgetsplugin.cc
  roadmap.cc
  linkwidget.cc

  LINK_DEPENDENCIES
  hppwidgetsplugin
  "hpp-manipulation-corba::hpp-manipulation-corba"
  "gepetto-viewer-corba::gepetto-viewer-corba"
  "Boost::thread" "Boost::regex" "Boost::system"
  )
