# TabsPls Core
This is the shared code between the main application and the test executable.

## C++ standard
At least c++17 is required.

## How to build
The shared code can not be built on its own. 
Using the core means an implementation for accessing the filesystem must be provisioned.

There is a `FileSystem.hpp` but no corresponding `.cpp` within the core itself. Currently the main app provisions this with std::filesystem calls. The test applications provisions this using a fake filesystem (which is nice to test core logic within a sandbox).
`FileSystem.hpp` also includes `FileSystemDefs.hpp` which must also be provisioned by the implementation project. 
`FileSystemDefs.hpp` defines which string types are used to represent paths and other file system primitives. Currently, wide strings are used for any platform. But if this needs to change then the required changes are contained.
