# TabsPls Core
This is the shared code for any flavour of the Light Speed File Explorer.

## C++ standard
At least c++17 is required.

## How to build
The shared code can not be built on its own. 
Each individual flavour of Light Speed File Explorer must provision the core with an implementation for accessing the file system.

There is a `FileSystem.hpp` but no corresponding `.cpp` within the core itself. This must be provided by the flavour project.
`FileSysten.hpp` also includes `FileSystemDefs.hpp` which must also be provisioned by the flavour project. 
`FileSystemDefs.hpp` defines which string types are used to represent paths and other file system primitives. For example the GTK flavour uses utf-8 strings, so `std::string` is useful here, whereas the UWP flavour uses wide-strings.