# Copyright (c) 2018, 2020 CNRS
# Authors: Joseph Mirabel, Guilhem Saurel
#
#
# This file is part of gepetto-viewer-corba
# gepetto-viewer-corba is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# gepetto-viewer-corba is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# gepetto-viewer-corba  If not, see
# <http://www.gnu.org/licenses/>.

IF(USE_QT4)
  SET(QT4 "QT4")
ELSE()
  SET(QT4 "")
ENDIF()

GEPETTO_GUI_PLUGIN(hppfclplugin
  ${QT4}
  HEADERS_NO_MOC
  node.hh

  HEADERS
  plugin.hh
  FORMS

  RESOURCES

  SOURCES
  plugin.cc
  node.cc

  LINK_DEPENDENCIES
  hpp-fcl::hpp-fcl
  gepetto-viewer-corba::gepetto-viewer-corba
  Boost::thread Boost::regex Boost::system

  PKG_CONFIG_DEPENDENCIES
  )
