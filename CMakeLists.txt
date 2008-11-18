project(LatticeGasExplorer)

cmake_minimum_required(VERSION 2.6)

#-----------------

# if you don't have multiple CPUs you can use OpenMP to get the program to run faster
OPTION(USE_OMP "Use OpenMP" ON)

IF(USE_OMP)
    IF(UNIX)
        IF(USE_ICC)
            #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp -axT")
            #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -openmp -axT")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp")
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -openmp")
        ELSE(USE_ICC)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
            #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse")
            #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpmath=sse")
        ENDIF(USE_ICC)
    ELSE(UNIX)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /openmp")
    ENDIF(UNIX)
    ADD_DEFINITIONS(-D USE_OMP)
ENDIF(USE_OMP)

#-----------------------------------------------------------------------------

FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})
LINK_LIBRARIES(${wxWidgets_LIBRARIES})

#-----------------------------------------------------------------------------

ADD_EXECUTABLE(LatticeGasExplorer
  WIN32
  src/lga.cpp
)

install(TARGETS LatticeGasExplorer RUNTIME DESTINATION bin)
#------------------------------------------------------------------------------

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lattice Gas Explorer")
SET(CPACK_PACKAGE_VENDOR "Tim Hutton <tim.hutton@gmail.com>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/gpl.txt")
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