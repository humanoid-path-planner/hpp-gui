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

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CXX_DISABLE_WERROR TRUE)

INCLUDE(cmake/base.cmake)

SET(PROJECT_NAME "hpp-gui")
SET(PROJECT_URL "https://github.com/jmirabel/hpp-gui")
SET(PROJECT_DESCRIPTION "Graphical interface for HPP ")
SET(PROJECT_VERSION "0")

SETUP_PROJECT()

FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGl QtNetwork)

ADD_REQUIRED_DEPENDENCY("openscenegraph >= 3.2")
ADD_REQUIRED_DEPENDENCY("openscenegraph-osgQt")
ADD_REQUIRED_DEPENDENCY("openthreads >= 3.2")
ADD_REQUIRED_DEPENDENCY("hpp-corbaserver")
ADD_REQUIRED_DEPENDENCY("gepetto-viewer-corba")

ADD_OPTIONAL_DEPENDENCY("remoteimu")

FIND_PACKAGE("X11")

# Activate hpp-util logging if requested
SET (HPP_DEBUG FALSE CACHE BOOL "trigger hpp-util debug output")
IF (HPP_DEBUG)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHPP_DEBUG")
ENDIF()

SET (${PROJECT_NAME}_HEADERS
  include/hpp/gui/windows-manager.h
  include/hpp/gui/mainwindow.h
  include/hpp/gui/osgwidget.h
  include/hpp/gui/configurationlistwidget.h
  include/hpp/gui/pathplayer.h
  include/hpp/gui/solverwidget.h
  include/hpp/gui/tree-item.h
  include/hpp/gui/ledindicator.h
  include/hpp/gui/plugin-interface.h
  include/hpp/gui/omniorb/omniorbthread.h
  include/hpp/gui/dialog/dialogloadrobot.h
  include/hpp/gui/dialog/dialogloadenvironment.h
  include/hpp/gui/dialog/pluginmanagerdialog.h
  )

SET(${PROJECT_NAME}_FORMS
  include/hpp/gui/ui/mainwindow.ui
  include/hpp/gui/ui/dialogloadrobot.ui
  include/hpp/gui/ui/dialogloadenvironment.ui
  include/hpp/gui/ui/pluginmanagerdialog.ui
  )

SET(${PROJECT_NAME}_RESOURCES res/images.qrc)

QT4_WRAP_CPP(${PROJECT_NAME}_HEADERS_MOC ${${PROJECT_NAME}_HEADERS})
QT4_WRAP_UI(${PROJECT_NAME}_FORMS_HEADERS ${${PROJECT_NAME}_FORMS})
QT4_ADD_RESOURCES(${PROJECT_NAME}_RESOURCES_RCC ${${PROJECT_NAME}_RESOURCES})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

CONFIGURE_FILE (src/main.cc
  ${CMAKE_BINARY_DIR}/src/main.cc
  @ONLY
)

SET (${PROJECT_NAME}_SOURCES
  ${CMAKE_BINARY_DIR}/src/main.cc
  src/windows-manager.cc
  src/mainwindow.cc
  src/configurationlistwidget.cc
  src/osgwidget.cc
  src/tree-item.cc
  src/pathplayer.cc
  src/solverwidget.cc
  src/omniorbthread.cc
  src/ledindicator.cc
  src/dialog/dialogloadrobot.cc
  src/dialog/dialogloadenvironment.cc
  src/dialog/pluginmanagerdialog.cc
  )

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS_MOC}
  ${${PROJECT_NAME}_FORMS_HEADERS}
  ${${PROJECT_NAME}_RESOURCES_RCC}
  )

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} openscenegraph)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} openthreads)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} hpp-corbaserver)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} gepetto-viewer-corba)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} openscenegraph-osgQt)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${X11_LIBRARIES})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

ADD_SUBDIRECTORY(plugins)

SETUP_PROJECT_FINALIZE()
