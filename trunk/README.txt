=== Build ===

Required install:
- wxWidgets
- CMake

Optional install:
- OpenMP

Build instructions:
- run CMake, build. If you have problems then read the CMake instructions. http://www.cmake.org

Build instructions on Windows:
- run CMakeSetup.exe, configure project
- open the generated IDE file (eg. *.sln) in your compiler's IDE
- build

Build instructions on Linux:
- run "ccmake ." 
- run "make"

== TODO ==

- implement more efficiently (spin coding in Wolf-Gladrow? rule tree?)
- make a toolbar to select gas types and demos
- make a toolbar to paint cell types
- improve graphics to better show multiple occupancy
