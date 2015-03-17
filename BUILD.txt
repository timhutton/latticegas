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

- allow image panning (redraw efficiently just the bit we need)
- report its/sec
- different boundary conditions: slip (get odd boundary effects currently, e.g. PI, suspect bug)
- re-create figures from books as a check: 
    - Fig. 3.6.5 from Wolf-Gladrow (p. 120) (PI-LGA)
    - Fig. 3.2.8 from ibid. (p. 83) (FHP-II)
    - Figs. on pages 378 and 380 of NKS (FHP6)
- implement more efficiently (spin coding in Wolf-Gladrow? rule tree? bool[N_DIRS] instead of bits?)
- make a toolbar to select gas types and demos, start/stop, etc.
- make a toolbar to paint cell types
- improve graphics to better show multiple occupancy

- i18n
- show user information about each gas? (history, references, links)
- show user the collision classes for each gas type?
- allow user to control: input flow speed, density, world size
- step backwards?
