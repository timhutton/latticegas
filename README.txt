=== Build ===

Required install:
- wxWidgets
- CMake

Optional install:
- OpenMP

Build instructions:
- run CMake, build. If you have problems then read the CMake instructions. http://www.cmake.org

Build instructions on Windows:
- run CMakeSetup.exe
- open the generated IDE file (eg. *.sln) in your compiler's IDE
- build
(Or you might find it easier to start with wxWidgets/samples/minimal.dsp and swap-in lga.cpp)

Build instructions on Linux:
- run "ccmake ." 
- run "make"
