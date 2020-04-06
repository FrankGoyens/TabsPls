# TabsPls
A modern approach to a file system explorer, can we have some tabs please?

## Why use this?
Because you're done with Windows' built in file explorer that still does not support tabs to this date.
You also aren't satisfied using a conventional terminal emulator with nice and smooth tab completion. This currently also does not exist on Windows, and this does not integrate well with other tooling. You can't drag and drop files for example.

## Dependencies:
* Gtk+3
* gtest (only needed for testing)
* CMake
* PkgConfig (optional, for finding Gtk+3)

## C++ standard
At least c++17 is required.

## Build environment on Linux
This should be trivial to set up on Linux. Just download the dependencies using you favourite package manager and let CMake do the rest.

## Windows build environment
This was mainly to replace Windows' built in file explorer in my workflow, so I've developed this mainly on Windows. MSYS2 is the fastest to get up and running due to its package manager, but debugging is hard. Not because gdb is hard, but because gdb runs terribly on Windows (mileage may vary). 

I have not tested this with a clang toolchain, this might work better, try it if you're curious.

Visual Studio is also supported, I prefer this one, but it's not as easy to set up.

### Option1: using [MSYS2](https://www.msys2.org/).
1. Open MSYS2 terminal
2. Install dependencies using pacman
3. Clone this repository
4. Run `cmake -G "MSYS2 Makefiles" -DCMAKE_MAKE_PROGRAM="mingw32-make" <source_dir>`
5. Run `cmake --build .`
6. Done!

### Option2: using Visual Studio + vcpkg
1. Open Powershell
2. Navigate to where you have vcpkg installed
3. Install dependencies 
4. Create a folder where you want to build TabsPls
5. In that build folder run `cmake <source_dir>`. note that VStudio's CMake project integration is NOT used, together with vcpkg this is a PITA.
6. Open the resulting `sln` file, or if you want to build from command line run `cmake --build . --config <Debug|Release|...>` and skip to step 9
7. Choose the right build type
8. Click Build>Build Solution (ctrl+shift+b)
9. Done!