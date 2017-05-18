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

SET(PLUGIN_NAME plugintest)

SET(${PLUGIN_NAME}_HEADERS_NO_MOC
  ../../include/hpp/gui/plugin-interface.hh
  )

SET(${PLUGIN_NAME}_HEADERS
  test.hh
  )

SET(${PLUGIN_NAME}_FORMS
  )

SET(${PLUGIN_NAME}_RESOURCES
  )

QT4_WRAP_CPP(${PLUGIN_NAME}_HEADERS_MOC ${${PLUGIN_NAME}_HEADERS} OPTIONS -I${GEPETTO_VIEWER_CORBA_INCLUDEDIR})
QT4_WRAP_UI(${PLUGIN_NAME}_FORMS_HEADERS ${${PLUGIN_NAME}_FORMS})
QT4_ADD_RESOURCES(${PLUGIN_NAME}_RESOURCES_RCC ${${PLUGIN_NAME}_RESOURCES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

SET (${PLUGIN_NAME}_SOURCES
  test.cc
  )

ADD_LIBRARY(${PLUGIN_NAME} SHARED ${${PLUGIN_NAME}_SOURCES}
  ${${PLUGIN_NAME}_HEADERS_NO_MOC}
  ${${PLUGIN_NAME}_HEADERS_MOC}
  ${${PLUGIN_NAME}_FORMS_HEADERS}
  ${${PLUGIN_NAME}_RESOURCES_RCC}
  )

TARGET_LINK_LIBRARIES(${PLUGIN_NAME} ${QT_LIBRARIES})

# Uncomment to install
#INSTALL(TARGETS ${PLUGIN_NAME} DESTINATION lib)
