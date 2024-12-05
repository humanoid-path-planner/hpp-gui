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

cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "hpp-gui")
set(PROJECT_DESCRIPTION "Graphical interface for HPP ")

set(PROJECT_USE_CMAKE_EXPORT TRUE)
set(CXX_DISABLE_WERROR TRUE)

# Check if the submodule cmake have been initialized
set(JRL_CMAKE_MODULES "${CMAKE_CURRENT_LIST_DIR}/cmake")
if(EXISTS "${JRL_CMAKE_MODULES}/base.cmake")
  message(STATUS "JRL cmakemodules found in 'cmake/' git submodule")
else()
  find_package(jrl-cmakemodules QUIET CONFIG)
  if(jrl-cmakemodules_FOUND)
    get_property(
      JRL_CMAKE_MODULES
      TARGET jrl-cmakemodules::jrl-cmakemodules
      PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "JRL cmakemodules found on system at ${JRL_CMAKE_MODULES}")
  elseif(${CMAKE_VERSION} VERSION_LESS "3.14.0")
    message(
      FATAL_ERROR
        "\nCan't find jrl-cmakemodules. Please either:\n"
        "  - use git submodule: 'git submodule update --init'\n"
        "  - or install https://github.com/jrl-umi3218/jrl-cmakemodules\n"
        "  - or upgrade your CMake version to >= 3.14 to allow automatic fetching\n"
    )
  else()
    message(STATUS "JRL cmakemodules not found. Let's fetch it.")
    include(FetchContent)
    FetchContent_Declare(
      "jrl-cmakemodules"
      GIT_REPOSITORY "https://github.com/jrl-umi3218/jrl-cmakemodules.git")
    FetchContent_MakeAvailable("jrl-cmakemodules")
    FetchContent_GetProperties("jrl-cmakemodules" SOURCE_DIR JRL_CMAKE_MODULES)
  endif()
endif()

include("${JRL_CMAKE_MODULES}/hpp.cmake")
include("${JRL_CMAKE_MODULES}/boost.cmake")

compute_project_args(PROJECT_ARGS LANGUAGES CXX)
project(${PROJECT_NAME} ${PROJECT_ARGS})

add_project_dependency("gepetto-viewer-corba" REQUIRED)

# Get desired Qt version
string(REPLACE "." ";" DESIRED_QT_VERSION_LIST
               ${GEPETTO_VIEWER_CORBA_QTVERSION})
list(GET DESIRED_QT_VERSION_LIST 0 DESIRED_QT_VERSION_MAJOR)

if(${DESIRED_QT_VERSION_MAJOR} EQUAL 4)
  set(USE_QT4 True)
elseif(${DESIRED_QT_VERSION_MAJOR} EQUAL 5)
  set(USE_QT4 False)
else()
  message(FATAL_ERROR "This package is only compatible with Qt 4 and Qt 5")
endif()
message(STATUS "Looking for Qt ${DESIRED_QT_VERSION_MAJOR}.")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(USE_QT4)
  find_package(Qt4 REQUIRED QtCore QtGui QtOpenGl QtNetwork)
  set(HPP_GUI_QTVERSION
      "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
else(USE_QT4)
  find_package(
    Qt5 REQUIRED
    COMPONENTS Core
               Widgets
               Gui
               OpenGL
               Concurrent
               Network
               Xml)
  foreach(
    component
    "Core"
    "Widgets"
    "Gui"
    "OpenGL"
    "Concurrent"
    "Network"
    "Xml")
    list(APPEND QT_INCLUDE_DIRS ${Qt5${component}_INCLUDE_DIRS})
    list(APPEND QT_LIBRARIES ${Qt5${component}_LIBRARIES})
  endforeach(component "Core" "Widgets" "Gui" "OpenGL" "Network")
  set(HPP_GUI_QTVERSION "${Qt5Core_VERSION}")
endif(USE_QT4)
set(PKG_CONFIG_EXTRA "qtversion=${HPP_GUI_QTVERSION}")
set(PACKAGE_EXTRA_MACROS "set(HPP_GUI_QTVERSION ${HPP_GUI_QTVERSION})")

# Search for Boost.
add_project_dependency(Boost REQUIRED COMPONENTS thread regex system)

add_project_dependency("coal")
add_project_dependency("hpp-manipulation-corba")
add_optional_dependency("remoteimu")

add_library(${PROJECT_NAME} INTERFACE)
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${TARGETS_EXPORT_NAME}
  DESTINATION lib)

# This line allows QT-MOC to compile with the include from gepetto-gui It cannot
# be done through the pkg-config framework because the moc settings are
# generated before PKG_CONFIG_USE_DEPENDENCY calls. Without this, file
# gepetto/gui/plugin-interface.hh is not found.
include_directories(${GEPETTO_VIEWER_CORBA_INCLUDE_DIRS})

add_subdirectory(plugins)
add_subdirectory(pyplugins)

# If settings.conf not yet installed, install it
if(EXISTS
   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf"
)
  message(
    STATUS
      "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf detected."
  )
else()
  message(
    STATUS
      "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf does not exist. Will install it."
  )
  install(FILES etc/gepetto-gui/settings.conf
          DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui)
endif()

# If basic.conf not yet installed, install it
if(EXISTS
   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf")
  message(
    STATUS
      "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf detected."
  )
else()
  message(
    STATUS
      "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf does not exist. Will install it."
  )
  install(FILES etc/gepetto-gui/basic.conf
          DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui)
endif()

install(FILES package.xml DESTINATION share/${PROJECT_NAME})
