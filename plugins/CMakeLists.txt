#
# Copyright (c) 2015 CNRS
# Authors: Joseph Mirabel
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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(BUILD_REMOTEIMU_PLUGIN       ${REMOTEIMU_FOUND}
  CACHE BOOL "Build remoteimuplugin")
SET(BUILD_HPP_CORBASERVER_PLUGIN ${HPP_CORBASERVER_FOUND}
  CACHE BOOL "Build hppcorbaserverplugin")
SET(BUILD_HPP_WIDGETS_PLUGIN     ${HPP_CORBASERVER_FOUND}
  CACHE BOOL "Build hppwidgetsplugin")
SET(BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN ${HPP_MANIPULATION_CORBA_FOUND}
  CACHE BOOL "Build hppmanipulationwidgetsplugin")

#ADD_SUBDIRECTORY(plugintest)
IF (HPP_CORBASERVER_FOUND)
  IF (REMOTEIMU_FOUND AND BUILD_REMOTEIMU_PLUGIN)
    ADD_SUBDIRECTORY(remoteimuplugin)
  ENDIF (REMOTEIMU_FOUND AND BUILD_REMOTEIMU_PLUGIN)

  IF (BUILD_HPP_CORBASERVER_PLUGIN)
    ADD_SUBDIRECTORY(hppcorbaserverplugin)
  ENDIF (BUILD_HPP_CORBASERVER_PLUGIN)

  IF (BUILD_HPP_WIDGETS_PLUGIN)
    ADD_SUBDIRECTORY(hppwidgetsplugin)
  ENDIF (BUILD_HPP_WIDGETS_PLUGIN)

  IF (HPP_MANIPULATION_CORBA_FOUND AND BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN)
    ADD_SUBDIRECTORY(hppmanipulationwidgetsplugin)
  ENDIF (HPP_MANIPULATION_CORBA_FOUND AND BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN)
ENDIF (HPP_CORBASERVER_FOUND)
