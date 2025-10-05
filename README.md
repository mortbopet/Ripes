# Ripes
[![Windows / Qt 6.5.0](https://github.com/mortbopet/Ripes/actions/workflows/windows-release.yml/badge.svg)](https://github.com/mortbopet/Ripes/actions/workflows/windows-release.yml)
[![Mac release / Qt 6.5.0](https://github.com/mortbopet/Ripes/actions/workflows/mac-release.yml/badge.svg)](https://github.com/mortbopet/Ripes/actions/workflows/mac-release.yml)
[![Ubuntu release 16.04 / Qt 6.5.0](https://github.com/mortbopet/Ripes/actions/workflows/linux-release.yml/badge.svg)](https://github.com/mortbopet/Ripes/actions/workflows/linux-release.yml)
[![WASM / Qt 6.6.0](https://github.com/mortbopet/Ripes/actions/workflows/wasm-release.yml/badge.svg?branch=master)](https://github.com/mortbopet/Ripes/actions/workflows/wasm-release.yml)
[![Gitter](https://badges.gitter.im/Ripes-VSRTL/Ripes.svg)](https://gitter.im/Ripes-VSRTL/)

Ripes is a visual computer architecture simulator and assembly code editor built for the [RISC-V instruction set architecture](https://content.riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf).

**Experimental**: Try Ripes directly in your browser: [https://ripes.me/](https://ripes.me/)

If you enjoy using Ripes, or find it useful in teaching, feel free to leave a tip through [Ko-Fi](https://ko-fi.com/mortbopet).

For questions, comments, feature requests, or new ideas, don't hesitate to share these at the [discussions page](https://github.com/mortbopet/Ripes/discussions).  
For bugs or issues, please report these at the [issues page](https://github.com/mortbopet/Ripes/issues).

<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/images/animation.gif?raw=true" />
</p>

## Usage
Ripes may be used to explore concepts such as:
- How machine code is executed on a variety of microarchitectures (RV32IMC/RV64IMC based)
- How different cache designs influence performance
- How C and assembly code is compiled and assembled to executable machine code
- How a processor interacts with memory-mapped I/O

If this is your first time using Ripes, please refer to the [introduction/tutorial](docs/introduction.md).  
For further information, please refer to the [Ripes documentation](docs/README.md).

## Downloading & Installation
Prebuilt binaries are available for Linux, Windows & Mac through the [Releases page](https://github.com/mortbopet/Ripes/releases).  

### Linux
Releases for Linux are distributed in the AppImage format. To run an AppImage:
* Run `chmod a+x` on the AppImage file
* Run the file!
The AppImage for Linux should be compatible with most Linux distributions.

### Windows
For Windows, the C++ runtime library must be available (if not, a msvcp140.dll error will be produced). You most likely already have this installed, but if this is not the case, you download it [here](https://www.microsoft.com/en-us/download/details.aspx?id=48145).

## Building
Initially, the following dependencies must be made available:
- A recent (>=6.5.0) version of [Qt](https://www.qt.io/download) + Qt Charts (**not** bundled with Qt by default, but can be selected during Qt installation)
- [CMake](https://cmake.org/)
- `sudo apt-get install libegl1-mesa-dev`

Then, Ripes can be checked out and built as a standard CMake project:
```
git clone --recursive https://github.com/mortbopet/Ripes.git
cd Ripes/
cmake .
Unix:               Windows:
make                jom.exe / nmake.exe / ...
```
Note, that you must have Qt available in your `CMAKE_PREFIX_PATH`. For further information on building Qt projects with CMake, refer to [Qt: Build with CMake](https://doc.qt.io/qt-5/cmake-manual.html).

---
In papers and reports, please refer to Ripes as follows: 'Morten Borup Petersen. Ripes. https://github.com/mortbopet/Ripes' or by referring to the [WCAE'21 paper on the project](https://ieeexplore.ieee.org/document/9707149), e.g. using the following BibTeX code:
```
@MISC{Ripes,
	author = {Morten Borup Petersen},
	title = {Ripes},
	howpublished = "\url{https://github.com/mortbopet/Ripes}"
}

@inproceedings{petersen2021ripes,
  title={Ripes: A Visual Computer Architecture Simulator},
  author={Petersen, Morten B},
  booktitle={2021 ACM/IEEE Workshop on Computer Architecture Education (WCAE)},
  pages={1--8},
  year={2021},
  organization={IEEE}
}
```
### Implementation of moodle with ripes interaction, configuration of connection to service from moodle side.
Configuring the External Tool it is done through received editing rights then add an external tool for subsequent configuration.
<p align="center">
    <img src="https://github.com/moevm/mse1h2024ripes/tree/master/resources/images/externaltool.jpg?raw=true" />
</p>
Add name, URL and select LTI version.
<p align="center">
    <img src="https://github.com/moevm/mse1h2024ripes/tree/master/resources/images/nameurl.jpg?raw=true" />
</p>
That’s it.
<p align="center">
    <img src="https://github.com/moevm/mse1h2024ripes/tree/master/resources/images/reslti.jpg?raw=true" />
</p>



