# Ripes
[![Build Status](https://travis-ci.org/mortbopet/Ripes.svg?branch=master)](https://travis-ci.org/mortbopet/Ripes/)
[![Gitter](https://badges.gitter.im/Ripes-VSRTL/Ripes.svg)](https://gitter.im/Ripes-VSRTL/)

Ripes is a graphical processor simulator and assembly code editor built for the [RISC-V instruction set architecture](https://content.riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf), suitable for teaching how assembly level code is executed on various microarchitectures.

Got questions or comments? Head over to the Ripes [Gitter chat](https://gitter.im/Ripes-VSRTL/).  
Report bugs, issues or feature requests at https://github.com/mortbopet/Ripes/issues.

<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/images/animation.gif?raw=true" />
</p>

## Usage
If this is your first time using Ripes, please refer to the [introduction](https://github.com/mortbopet/Ripes/wiki/(2.0)-Introduction).  
For further information, please refer to the [Ripes wiki](https://github.com/mortbopet/Ripes/wiki).

## Downloading & Installation
Prebuilt binaries are available for Linux, Windows & Mac. These are available through the [Releases page](https://github.com/mortbopet/Ripes/releases).  
**It is recommended to use the *[continuous builds](https://github.com/mortbopet/Ripes/releases/tag/continuous)*, since they contain all of the most-recent bug-fixes.**

### Linux
Releases for Linux are distributed in the AppImage format. To run an AppImage:
* Run `chmod a+x` on the AppImage file
* Run the file!
The AppImage for Linux should be compatible with most Linux distributions.

### Windows
For Windows, the C++ runtime library must be available (if not, a msvcp140.dll error will be produced). You most likely already have this installed, but if this is not the case, you download it [here](https://www.microsoft.com/en-us/download/details.aspx?id=48145).

## Building
Assuming you have installed a recent version of [Qt](https://www.qt.io/download) as well as [CMake](https://cmake.org/), Ripes may be built like any other CMake project:
```
Unix:               Windows:
cd Ripes/           cd Ripes/
cmake .             cmake .
make                jom.exe / nmake.exe / ...
```
Note, that you must have Qt 5 available in your `CMAKE_PREFIX_PATH`. The easiest way to use CMake with Qt is to set the `CMAKE_PREFIX_PATH` environment variable to the install prefix of Qt 5.

---
Icons kindly provided by Flaticon authors: [Smashicons](https://www.flaticon.com/authors/smashicons), [Freepik](https://www.flaticon.com/authors/freepik), [Vectors Market](https://www.flaticon.com/authors/vectors-market) & [Pixel Buddha](https://www.flaticon.com/authors/pixel-buddha).

<a href="https://www.qt.io/">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/images/QtIcon.png" width="60" height="60" />
</a>
