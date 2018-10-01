#
#    Lattice Gas Explorer
#    Copyright (C) 2008-2009 Tim J. Hutton <tim.hutton@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

project(LatticeGasExplorer)

cmake_minimum_required(VERSION 2.6)
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

#-----------------

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#-----------------------------------------------------------------------------

# Here you can define what libraries of wxWidgets you need for your
# application. You can figure out what libraries you need here;
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html
IF(WIN32)
  SET(wxWidgets_USE_LIBS base core html)
ENDIF(WIN32)


FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})
LINK_LIBRARIES(${wxWidgets_LIBRARIES})

#-----------------------------------------------------------------------------

ADD_EXECUTABLE(LatticeGasExplorer
  WIN32
  src/lga.cpp
  src/BaseLatticeGas.cpp
  src/BaseLatticeGas.h
  src/BaseLatticeGas_drawable.cpp
  src/BaseLatticeGas_drawable.h
  src/SquareGridLatticeGas.cpp
  src/SquareGridLatticeGas.h
  src/HPPLatticeGas.cpp
  src/HPPLatticeGas.h
  src/PairInteractionLatticeGas.cpp
  src/PairInteractionLatticeGas.h
  src/HexGridLatticeGas.cpp
  src/HexGridLatticeGas.h
  src/FHPLatticeGas.cpp
  src/FHPLatticeGas.h
  src/LatticeGasFactory.cpp
  src/LatticeGasFactory.h
  src/wxWidgetsPreamble.h
)

install(TARGETS LatticeGasExplorer RUNTIME DESTINATION bin)
#------------------------------------------------------------------------------

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lattice Gas Explorer")
SET(CPACK_PACKAGE_VENDOR "Tim Hutton <tim.hutton@gmail.com>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "LatticeGasExplorer-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "LatticeGasExplorer-src")
SET(CPACK_SOURCE_GENERATOR "ZIP")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\LatticeGasExplorer.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Lattice Gas Explorer")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.sq3.org.uk")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.sq3.org.uk")
  SET(CPACK_NSIS_CONTACT "tim.hutton@gmail.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  #SET(CPACK_STRIP_FILES "bin/LatticeGasExplorer")
  #SET(CPACK_SOURCE_STRIP_FILES "TRUE")
  #SET(CPACK_SOURCE_GENERATOR "ZIP")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "LatticeGasExplorer" "Lattice Gas Explorer")
INCLUDE(CPack)
