# Build overview

The server-side tools can be built for Linux, macOS, and Windows.
The client-side rig2c* tools can be built for Linux, macOS, Windows, iOS, and Android. The basic steps to build are:
 - Install build tools
 - clone the git repository
 - run cmake
 - run the build tool (make, visual studio, etc.)

All output files are placed in the repository's `bin` folder. [kp2rig](/doc/kp2rig.md) requires [additional setup](/doc/kp2rig.md) to run.

### Python
An installation of python-dev is optional and not required for Blender because Blender provides a local installation.
If you don't need/want python support then add the following cmake argument:
  - `-DWITH_PYTHON=NO`

## Linux
### Requirements
 - [cmake 3.14 or newer](https://cmake.org/)
 - make
 - [gnu](https://gcc.gnu.org/) c++ toolchain, 6.1 or newer
 - [git](https://git-scm.com/)
 - [python-dev (optional)](https://www.python.org/) 

### Get the code
 - Open a terminal
 - `git clone <URL>`, where `<URL>` is the project's clone url
 - `cd riggingTools`

### Build all projects
 - `mkdir build`
 - `cd build`
 -`cmake -DCMAKE_BUILD_TYPE=Release ..`
   - `cmake -DCMAKE_BUILD_TYPE=Debug ..` for a debuggable build
 - `make`

## macOS
### Requirements
 - macOS computer with Xcode and Command Line Tools installed
 - [cmake 3.14 or newer](https://cmake.org/)
 - [Xcode](https://developer.apple.com/xcode/)
 - [git](https://git-scm.com/) (or use the built-in git packaged with Xcode)
 - [python-dev (optional)](https://www.python.org/) 

### Get the code
 - Open a terminal
 - `git clone <URL>`, where `<URL>` is the project's clone url
 - `cd riggingTools`

### Build all projects
 - `mkdir build`
 - `cd build`
 - `cmake -DCMAKE_BUILD_TYPE=Release ..`
   - `cmake -DCMAKE_BUILD_TYPE=Debug ..` for a debuggable build
   - `cmake -G Xcode .. -DCMAKE_OSX_ARCHITECTURES=<arch>` for an Xcode project (\<arch\> is your target architecture, I.E x86_64, arm64, etc)
 - `make`

## iOS
iOS only supports client-side utilities and not python.
### Requirements
 - macOS computer with Xcode and Command Line Tools installed
 - [cmake 3.16 or newer](https://cmake.org/)
 - [Xcode](https://developer.apple.com/xcode/)
 - [git](https://git-scm.com/) (or use the built-in git packaged with Xcode)

### Get the code
 - Open a terminal
 - `git clone <URL>`, where `<URL>` is the project's clone url
 - `cd riggingTools`

### Build all projects
 - `mkdir build`
 - `cd build`
 - `cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS`
 - Open the created project in Xcode
 - Select a Development Team for the target 'testIos': ![Set the Development Team](/img/testIos_devTeam.png)
 - Change the target device from 'My Mac' to the device/simulator you are building for (this won't build for macOS): ![Set the Target Device](/img/testIos_setDevice.png)
 - Add a Copy Bundle Resources phase to 'testIos': ![Add build phase](/img/testIos_addBuildPhase.png)
 - Drag-and-drop a rig file to this new build phase

## Android
--WIP--
Android only supports client-side utilities and not python.
### Requirements
 - [cmake 3.14 or newer](https://cmake.org/)
 - [NDK](https://developer.android.com/ndk)
 - make
 - [git](https://git-scm.com/)

### Get the code
 - Open a terminal
 - `git clone <URL>`, where `<URL>` is the project's clone url
 - `cd riggingTools`

### Build all projects
 - `mkdir build`
 - `cd build`
 - `cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=<ndk_path>/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=25`
   - `cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=<ndk_path>/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=25` for a debuggable build
 - `make`

## Windows
### Requirements
 - [cmake 3.14 or newer](https://cmake.org/)
 - [Visual Studio](https://visualstudio.microsoft.com/) 2017 or later
   - Ensure you have the 64-bit C++ development tools, sometimes this isn't installed by default
 - [git](https://git-scm.com/) (or use the built-in git packaged with Visual Studio)
 - [python-dev (optional)](https://docs.python.org/3/using/windows.html) 

### Get the code
 - Open a terminal
 - `git clone <URL>`, where `<URL>` is the project's clone url
 - `cd riggingTools`

### Build all projects
 - Open a Visual Studio Command Prompt. This is important because a normal command prompt will NOT have the environment set correctly.
   - This can be opened by starting Visual Studio and navigating the menu: Tools->Visual Studio Command Prompt
 - `mkdir build`
 - `cd build`
 - `cmake ..`
   - For a UWP build: `cmake -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION='10.0' ..`
 - `devenv riggingTools.sln /Build Release`
   - For a debug build: `devenv riggingTools.sln /Build Debug`

### Deploy
[VC++ Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) is required on target (non-development) PCs.
This is typical for applications released for Windows, often the installer packages this as part of the application.
