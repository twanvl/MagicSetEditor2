# Magic Set Editor 2

Magic Set Editor, or MSE for short, is a program with which you can design your own cards for popular trading card games. MSE can then generate images of those cards that you can print or upload to the internet. Magic Set Editor also has a statistics window that will give useful information about your set, like the average mana cost, number of rares, etc. When you have finished your set, you can export it to an HTML file to use on the Internet, or to Apprentice or CCG Lackey so you can play with your cards online.

More information on https://magicseteditor.boards.net/

## Dependencies

The code depends on
 * wxWidgets >= 3.0
 * boost
 * hunspell

## Building on windows with Visual Studio

On windows, the program can be compiled with Visual Studio (recommended) or with mingw-gcc.

 * Download and install [Visual Studio Community edition](https://visualstudio.microsoft.com/vs/community/)
 * Download and install [vcpkg](https://github.com/microsoft/vcpkg)
 * Use vcpkg to install wxwidgets, boost, hunspell
```shell
> vcpkg install wxwidgets
> vcpkg install boost-smart-ptr
> vcpkg install boost-regex
> vcpkg install boost-logic
> vcpkg install boost-pool
> vcpkg install hunspell
> vcpkg integrate install
```
 * Then just use "Open Folder" from inside visual studio to open the Magic Set Editor source code root folder.
 * Select the configuration that you want to build, and hit F7.

Notes:
 * You may need to work around [this bug](https://github.com/microsoft/vcpkg/issues/4756) by replacing `$VCPATH\IDE\CommonExtensions\Microsoft\CMake\CMake\share\cmake-3.16\Modules\FindwxWidgets.cmake` with the file from  https://github.com/CaeruleusAqua/vcpkg-wx-find (`$VCPATH` is usually `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7`)
 * vcpkg by default installs 32 bit versions of libraries, use `vcpkg install PACKAGENAME:x64-windows` if you want to enable a 64 bit build.
 * Similarly, to use a static build, use `vcpkg install PACKAGENAME:x32-windows-static`.
 
For running tests you will also need to
 * Download and install perl (For example [Strawberry perl](http://strawberryperl.com/) or using [MSYS2](https://www.msys2.org/))
The tests can be run from inside visual studio

## Building on windows with GCC (MSYS2)

 * Download and install [msys2](https://www.msys2.org/)
 * Install a recent version of the gcc compiler, cmake, and wxWidgets libraries:
```shell
> pacman -S mingw32/mingw-w64-i686-gcc
> pacman -S mingw32/mingw-w64-i686-wxWidgets
> pacman -S mingw32/mingw-w64-i686-boost
> pacman -S mingw32/mingw-w64-i686-hunspell
> pacman -S cmake
```
   Use `mingw64/mingw-w64-x86_64-...` instead of for the 64bit build
 * Build
```shell
> cmake -G "MSYS Makefiles" -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
> cmake --build build
```

## Building on linux

Install the dependencies, for example on a debian based system
```shell
$ sudo apt install g++ cmake
$ sudo apt install libboost-dev libboost-regex-dev libwxgtk3.0-gtk3-dev libhunspell-dev
```
Then use cmake to build
```shell
$ cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```
Use `CMAKE_BUILD_TYPE=Debug` for a debug build

## Building on Mac

Install the dependencies, for example using Homebrew
```shell
$ brew install boost wxwidgets hunspell cmake
```
Note: Tested with boost 1.72.0\_3, wxmac (wxwidgets) 3.0.5.1\_1, hunspell 1.7.0\_2
Then use cmake to build
```shell
$ cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```
Use `CMAKE_BUILD_TYPE=Debug` for a debug build

Finally, copy the resources to a SharedSupport directory and run the executable
```shell
$ mkdir SharedSupport && cd SharedSupport
$ cp -r ../resource SharedSupport/
$ ./magicseteditor
```
