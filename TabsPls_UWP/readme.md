# Light Speed File Explorer (UWP)
The GTK flavour of Light Speed File Explorer. This flavour targets the Universal Windows Platform.

## Dependencies
Visual Studio 2019 or later.

In the Visual Studio installer, make sure you have the following components:
* Desktop development with C++
* Universal Windows Platform development

## How to build
LightSpeedExplorer.sln is the main solution file. Visual Studio should know exactly how to build the app.

### Project structure
These are the individual projects in the solution.
#### LightSpeedExplorer (Universal Windows)
This is the front-end project made in C#. This is where the user interface is defined and bindings are made with the back-end data.
#### TabsPlsCoreUWP (Universal Windows)
This is the back-end project made in C++/WinRT. This project contains the TabsPls_Core sources and wraps the necessary objects with .idl objects. These .idl objects are the ones that can actually be used by the C# front-end.