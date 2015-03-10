#
# Copyright (c) 2015 CNRS
# Authors: Joseph Mirabel
#
#
# This file is part of hpp-manipulation
# hpp-manipulation is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# hpp-manipulation is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# hpp-manipulation  If not, see
# <http://www.gnu.org/licenses/>.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CXX_DISABLE_WERROR TRUE)

INCLUDE(cmake/base.cmake)

SET(PROJECT_NAME hpp-gui)
SET(PROJECT_URL "")
SET(PROJECT_DESCRIPTION "")

SETUP_PROJECT()

FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGl)

ADD_REQUIRED_DEPENDENCY("openscenegraph >= 3.2")
ADD_REQUIRED_DEPENDENCY("openthreads >= 3.2")
ADD_REQUIRED_DEPENDENCY("hpp-corbaserver")

# Activate hpp-util logging if requested
SET (HPP_DEBUG FALSE CACHE BOOL "trigger hpp-util debug output")
IF (HPP_DEBUG)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHPP_DEBUG")
ENDIF()

SET (${PROJECT_NAME}_HEADERS
  mainwindow.h
  osgwidget.h
  hpp-qt/corbaserver.hh
  )

SET (${PROJECT_NAME}_SOURCES
  main.cc
  mainwindow.cc
  osgwidget.cc
  hpp-qt/corbaserver.cc
  )

SET(${PROJECT_NAME}_FORMS mainwindow.ui)

QT4_WRAP_CPP(${PROJECT_NAME}_HEADERS_MOC ${${PROJECT_NAME}_HEADERS})
QT4_WRAP_UI(${PROJECT_NAME}_FORMS_HEADERS ${${PROJECT_NAME}_FORMS})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} 
  ${${PROJECT_NAME}_HEADERS_MOC}
  ${${PROJECT_NAME}_FORMS_HEADERS}
  )

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} openscenegraph)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} openthreads)
PKG_CONFIG_USE_DEPENDENCY(${PROJECT_NAME} hpp-corbaserver)

SETUP_PROJECT_FINALIZE()
