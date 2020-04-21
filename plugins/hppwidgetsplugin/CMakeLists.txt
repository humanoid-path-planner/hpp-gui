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

INCLUDE(${GEPETTO_VIEWER_CMAKE_PLUGIN})

GEPETTO_GUI_PLUGIN(hppwidgetsplugin
  ${QT}

  HEADERS
  hppwidgetsplugin.hh
  pathplayer.hh
  solverwidget.hh
  jointtreewidget.hh
  configurationlistwidget.hh
  joint-tree-item.hh
  jointbounddialog.hh
  transformwidget.hh
  constraintwidget.hh
  transformconstraintwidget.hh
  iconstraint.hh
  numericalconstraintpicker.hh
  configurationlist.hh
  twojointsconstraint.hh
  joint-action.hh

  HEADERS_NO_MOC
  roadmap.hh
  conversions.hh
  listjointconstraint.hh

  FORMS
  pathplayerwidget.ui
  solverwidget.ui
  jointtreewidget.ui
  configurationlistwidget.ui
  constraintwidget.ui
  transformconstraintwidget.ui
  numericalconstraintpicker.ui

  SOURCES
  hppwidgetsplugin.cc
  pathplayer.cc
  solverwidget.cc
  jointtreewidget.cc
  configurationlistwidget.cc
  joint-tree-item.cc
  jointbounddialog.cc
  roadmap.cc
  transformwidget.cc
  constraintwidget.cc
  transformconstraintwidget.cc
  numericalconstraintpicker.cc
  configurationlist.cc
  twojointsconstraint.cc
  listjointconstraint.cc
  joint-action.cc

  LINK_DEPENDENCIES
  ${Boost_LIBRARIES}
  "hpp-corbaserver::hpp-corbaserver"
  "gepetto-viewer-corba::gepetto-viewer-corba"
  )
