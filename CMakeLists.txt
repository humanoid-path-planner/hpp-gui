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
INCLUDE(cmake/boost.cmake)
INCLUDE(cmake/python.cmake)

SET(PROJECT_NAME "hpp-gui")
SET(PROJECT_URL "https://github.com/jmirabel/hpp-gui")
SET(PROJECT_DESCRIPTION "Graphical interface for HPP ")

SET(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake-modules/
  ${CMAKE_MODULE_PATH})

SETUP_PROJECT()

FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGl)

ADD_REQUIRED_DEPENDENCY("gepetto-viewer-corba")

SET(BOOST_COMPONENTS program_options system thread regex)
SEARCH_FOR_BOOST ()

ADD_OPTIONAL_DEPENDENCY("hpp-corbaserver")
ADD_OPTIONAL_DEPENDENCY("hpp-manipulation-corba")
ADD_OPTIONAL_DEPENDENCY("remoteimu")

# Activate hpp-util logging if requested
SET (HPP_DEBUG FALSE CACHE BOOL "trigger hpp-util debug output")
IF (HPP_DEBUG)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHPP_DEBUG")
ENDIF()

# This line allow the MOC to compile with the include from gepetto-gui
INCLUDE_DIRECTORIES(${GEPETTO_VIEWER_CORBA_INCLUDE_DIRS})

ADD_SUBDIRECTORY(plugins)

SETUP_PROJECT_FINALIZE()
