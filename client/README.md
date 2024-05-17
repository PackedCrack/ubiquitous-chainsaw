**As of now this is Windows 10/11 only.**

# Configure for Visual Studio
Clone the repo and then simply use the CMake GUI to configure for Visual Studio 17 (or newer). Once configured generate a visual studio solution. 
Don't forget to set the 'chainsaw' project to "startup project".

# Configure for Clion
* Clone the repo to disk
* Open the repos root directory in Clion
* In the top right corner under "CMake Profiles" (likely says Debug) press the arrow to open the drop down menu.
* Press "Edit CMake Profiles..."
* Press Add (the '+' icon), shortcut Alt + Insert (default).
* Under "Toolchain" to the right. Select Visual Studio.
* Select Generator (I use ninja)
* Repeat the process for Debug, RelWithDebInfo and Release

## Removing Cppcheck as build dependency
Cppcheck can be removed as a custom build target if it causes troubles. Or in case you just don't want to install it..
* open root/src/CMakeLists.txt
* Remove line 5 - "add_dependencies(${MAIN_PROJECT} cppcheck)"

