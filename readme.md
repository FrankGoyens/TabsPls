# TabsPls
A modern approach to a file system explorer, can we have some tabs please?

## Why use this?
Because you're done with Windows' built in file explorer that still does not support tabs to this date.
You also aren't satisfied using a conventional terminal emulator with nice and smooth tab completion. This currently also does not exist on Windows, and this does not integrate well with other tooling. You can't drag and drop files for example.

## Dependencies:
* Gtk+3
* gtest (only needed for testing)
* CMake
* PkgConfig (for finding Gtk+3)

## C++ standard
At least c++17 is required.

## Build environment on Linux
This should be trivial to set up on Linux. Just download the dependencies using you favourite package manager and let CMake do the rest.

## Example Windows (my) build environment
This was mainly to replace Windows' built in file explorer in my workflow, so I've developed this mainly on Windows.

This was done using [MSYS2](https://www.msys2.org/).
1. Open MSYS2 terminal
2. Install dependencies using pacman
3. Clone this repository
4. Run `cmake -G "MSYS2 Makefiles" -DCMAKE_MAKE_PROGRAM="mingw32-make" <source_dir>`
5. Run `cmake --build .`
6. Done!