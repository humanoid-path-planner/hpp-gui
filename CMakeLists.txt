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

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(PROJECT_NAME "hpp-gui")
SET(PROJECT_DESCRIPTION "Graphical interface for HPP ")

SET(PROJECT_USE_CMAKE_EXPORT TRUE)
SET(CXX_DISABLE_WERROR TRUE)

INCLUDE(cmake/hpp.cmake)
INCLUDE(cmake/boost.cmake)
INCLUDE(cmake/python.cmake)

COMPUTE_PROJECT_ARGS(PROJECT_ARGS LANGUAGES CXX)
PROJECT(${PROJECT_NAME} ${PROJECT_ARGS})

FINDPYTHON()

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/find-external/TinyXML)
FIND_PACKAGE(TinyXML REQUIRED)

ADD_PROJECT_DEPENDENCY("gepetto-viewer-corba" REQUIRED)

# Get desired Qt version
string(REPLACE "." ";" DESIRED_QT_VERSION_LIST ${GEPETTO_VIEWER_CORBA_QTVERSION})
list(GET DESIRED_QT_VERSION_LIST 0 DESIRED_QT_VERSION_MAJOR)

IF(${DESIRED_QT_VERSION_MAJOR} EQUAL 4)
  SET(USE_QT4 True)
ELSEIF(${DESIRED_QT_VERSION_MAJOR} EQUAL 5)
  SET(USE_QT4 False)
ELSE()
  MESSAGE(FATAL_ERROR "This package is only compatible with Qt 4 and Qt 5")
ENDIF()
MESSAGE(STATUS "Looking for Qt ${DESIRED_QT_VERSION_MAJOR}.")

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
IF(USE_QT4)
  FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGl QtNetwork)
  SET(HPP_GUI_QTVERSION "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
ELSE(USE_QT4)
  FIND_PACKAGE (Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL Concurrent Network Xml)
  FOREACH (component "Core" "Widgets" "Gui" "OpenGL" "Concurrent" "Network" "Xml")
    LIST(APPEND QT_INCLUDE_DIRS ${Qt5${component}_INCLUDE_DIRS})
    LIST(APPEND QT_LIBRARIES    ${Qt5${component}_LIBRARIES}   )
  ENDFOREACH (component "Core" "Widgets" "Gui" "OpenGL" "Network")
  SET(HPP_GUI_QTVERSION "${Qt5Core_VERSION}")
ENDIF(USE_QT4)
SET(PKG_CONFIG_EXTRA "qtversion=${HPP_GUI_QTVERSION}")
SET(PACKAGE_EXTRA_MACROS "set(HPP_GUI_QTVERSION ${HPP_GUI_QTVERSION})")

# Search for Boost.
ADD_PROJECT_DEPENDENCY(Boost REQUIRED COMPONENTS thread regex system)

ADD_PROJECT_DEPENDENCY("hpp-fcl")
ADD_PROJECT_DEPENDENCY("hpp-manipulation-corba")
ADD_OPTIONAL_DEPENDENCY("remoteimu")

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)
INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${TARGETS_EXPORT_NAME} DESTINATION lib)

# This line allows QT-MOC to compile with the include from gepetto-gui
# It cannot be done through the pkg-config framework because the moc settings
# are generated before PKG_CONFIG_USE_DEPENDENCY calls.
# Without this, file
# gepetto/gui/plugin-interface.hh
# is not found.
INCLUDE_DIRECTORIES(${GEPETTO_VIEWER_CORBA_INCLUDE_DIRS})

ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(pyplugins)

# If settings.conf not yet installed, install it
IF (EXISTS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf")
  MESSAGE (STATUS
    "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf detected.")
ELSE ()
  MESSAGE (STATUS "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/settings.conf does not exist. Will install it.")
  INSTALL (FILES etc/gepetto-gui/settings.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui)
ENDIF ()

# If basic.conf not yet installed, install it
IF (EXISTS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf")
  MESSAGE (STATUS
    "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf detected.")
ELSE ()
  MESSAGE (STATUS "File ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui/basic.conf does not exist. Will install it.")
  INSTALL (FILES etc/gepetto-gui/basic.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gepetto-gui)
ENDIF ()
