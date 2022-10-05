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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(BUILD_REMOTEIMU_PLUGIN
    ${REMOTEIMU_FOUND}
    CACHE BOOL "Build remoteimuplugin")
# SET(BUILD_HPP_CORBASERVER_PLUGIN OFF CACHE BOOL "Build hppcorbaserverplugin")
set(BUILD_HPP_WIDGETS_PLUGIN
    ${hpp-corbaserver_FOUND}
    CACHE BOOL "Build hppwidgetsplugin")
set(BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN
    ${hpp-manipulation-corba_FOUND}
    CACHE BOOL "Build hppmanipulationwidgetsplugin")
set(BUILD_HPP_FCL_PLUGIN
    ${hpp-fcl_FOUND}
    CACHE BOOL "Build hppfclplugin")

if(hpp-corbaserver_FOUND)
  if(REMOTEIMU_FOUND AND BUILD_REMOTEIMU_PLUGIN)
    add_subdirectory(remoteimuplugin)
  endif(REMOTEIMU_FOUND AND BUILD_REMOTEIMU_PLUGIN)

  # Disable because header <gepetto/gui/omniorb/omniorbthread.hh> no more
  # accessible. IF(BUILD_HPP_CORBASERVER_PLUGIN)
  # ADD_SUBDIRECTORY(hppcorbaserverplugin) ENDIF(BUILD_HPP_CORBASERVER_PLUGIN)

  if(BUILD_HPP_WIDGETS_PLUGIN)
    add_subdirectory(hppwidgetsplugin)
  endif(BUILD_HPP_WIDGETS_PLUGIN)

  if(hpp-manipulation-corba_FOUND AND BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN)
    add_subdirectory(hppmanipulationwidgetsplugin)
  endif(hpp-manipulation-corba_FOUND AND BUILD_HPP_MANIPULATION_WIDGETS_PLUGIN)
endif(hpp-corbaserver_FOUND)

if(BUILD_HPP_FCL_PLUGIN AND hpp-fcl_FOUND)
  add_subdirectory(hppfcl)
endif()
