# Light Speed File Explorer GTK
The GTK flavour of Light Speed File Explorer. This flavour targets the GNU/Linux platform.

Note: this can also be built for Windows, but the UWP flavour has better Windows support.

## Dependencies:
* CMake
* Gtk+3
* PkgConfig (optional, for finding Gtk+3 without vcpkg)
* Skyr url
* tl-expected
* vcpkg (optional, for installing dependencies)
* gtest (only needed for testing)

## C++ standard
At least c++17 is required.

## Dependency discovery
CMake uses find_package for the following libraries:
* PkgConfig
* Gtk+3 if PkgConfig is found
* Skyr url
* tl-expected
* gtest

These can all be installed using [vcpkg](https://github.com/Microsoft/vcpkg).
Some of these might also be available from your system's package manager, I recommend installing as many as possible using your system's package manager.
You DO NOT NEED PkgConfig for Gtk+3 if you use vcpkg to install Gtk+3.

## Build environment on Linux with vcpkg
CMake, PkgConfig, Gtk+3 and gtest are commonly availble from you system's package manager.

For the other dependencies vcpkg is recommended.
Install [vcpkg](https://github.com/Microsoft/vcpkg) using the instructions on their page.
Using vcpkg install the dependencies:

`./vcpkg install skyr-url tl-expected`

Then, execute cmake with vcpkg's toolchain file:

`cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg toolchain file> <TabsPls_sourcedir>/TabsPls_GTK`

## Windows build environment with vcpkg
This was mainly to replace Windows' built in file explorer in my workflow, so I've developed this mainly on Windows. 

All dependencies except CMake should be installed using vcpkg.
So first install [vcpkg](https://github.com/Microsoft/vcpkg) using the instructions on their page.

1. Open Powershell
2. Navigate to where you have vcpkg installed
3. Install dependencies `./vcpkg install gtk gtest skyr-url tl-expected`
4. Create a folder where you want to build TabsPls
5. In that build folder run `cmake <source_dir>`. note that VStudio's CMake project integration is NOT used, together with vcpkg this is a PITA.
6. Open the resulting `sln` file, or if you want to build from command line run `cmake --build . --config <Debug|Release|...>` and skip to step 9
7. Choose the right build type
8. Click Build>Build Solution (ctrl+shift+b)
9. Done!
