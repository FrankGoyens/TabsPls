# Light Speed File Explorer
Also known by its working title, TabsPls.
A modern approach to a file system explorer, can we have some tabs please?

Light Speed File Explorer is a simple file explorer that offers tab support. There are many file explorers out there, most aim to be feature rich to the point where you don't need any other file management tools. The downside of that approach is that the GUI becomes cluttered, and the program becomes a _Jack of all trades, master of none_.

Simplicity is the aim, but Light Speed File Explorer is still made with power users in mind.

Maybe you don't prefer graphical file explorers/managers and instead use a terminal emulator. Those do not integrate well with other graphical user interface tooling however. You can't drag and drop for example, you also can't select files and folders to copy and paste.

## Hybrid solution
Light Speed File Explorer combines fast navigation with the convenience of a graphical user interface, from which you can drag files into emails, collaboration tools and more. Did I mention it also supports tabs? Because it does.

## Keyboard only navigation (should you prefer it)
The navigation is inspired by tab completion that you find in bash for example. This means that the goal is to be able to navigate without your hands leaving the keyboard. 

## Cross platform
Have the same file explorer on all platforms.

## Feature summary 
* Tab support
* Drag 'n Drop: Drag files and/or directories to other programs, or from other programs.
* Instant filter: no separate searching context. Just type, and only the matching files and directories remain.
* Directory flattening: recursively show all files under a directory.
* Address bar tab-completion: press tab while typing a directory address to append the next directory.
* Recycle: Send files/directories to the Trash (or Recycling bin) where you may still retrieve them.
* (Windows) Files have their corresponding icons.
* (Windows) Native context menu: have the windows file explorer's context menu.

## How to build
There are two buildable components. The main application and the test application.

### C++ standard
At least c++17 is required.

### Main app
Dependencies:
* cmake (for building)
* Qt 5.12+ Widgets and Linguisttools
* \<filesystem\> (from C++17 standard library)

Building:
1. Navigate to the folder where you want to build
2. In this folder execute `cmake <source_folder>`. **Note for vcpkg users**: If you installed Qt using vcpkg add the argument `-DCMAKE_PREFIX_PATH=<vcpkg_path>/installed/<build-triplet>`.
3. Execute `cmake --build . --config Release` (do not forget the dot)
4. Done!

### Running tests
Dependencies:
* cmake (for building)
* Google Test library

The tests are run in a fake filesystem, so don't worry about actual files being messed with. 

Building:
1. Navigate to the folder where you want to build
2. In this folder execute `cmake <source_folder>`. 
3. Execute `cmake --build . --config Release` (do not forget the dot)
4. Execute `ctest . -C Release` (do not forget the dot)
4. Done!
