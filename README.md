## Ripes
[![Build Status](https://travis-ci.org/mortbopet/Ripes.svg?branch=master)](https://travis-ci.org/mortbopet/Ripes/)

Ripes is a graphical 5-stage processor pipeline simulator and assembly code editor built for the [RISC-V instruction set architecture](https://content.riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf), suitable for teaching how assembly level code is executed on a classic pipelined architecture. Ripes is especially suitable for illustrating how concepts such as forwarding and stalling works, giving a visual representation of both cases.
<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/animation.gif?raw=true" />
</p>

## Features
* **Simulator**:
  * 5-stage graphical pipeline simulator with visualization for multiplexer input selection, register write/enable signals and more.
  * Stage information: Current stage instructions are displayed above each stage, as well as in the instruction memory view
  * Breakpoint setting
  * Supports RISC-V assembly files & RISC-V binary files (extracted *.text* segments)
* **Memory**:
  * Display type selection for various display formats (Hex, Decimal, ...)
  * Memory view with features such as scrolling adress saving and adress jumping
* **Editor**:
  * Syntax highlighting and syntax error information
  * Real-time assembler for side-by-side comparison of assembly code and (disassembled) binary code

## Downloading & installation
No installation is required - all required libraries are packaged with the compiled executable.
Prebuilt versions for Windows and Ubuntu are available at the [Releases page](https://github.com/mortbopet/Ripes/releases).

For Windows, the C++ runtime library must be available (if not, a msvcp140.dll error will be produced). You can download it [here](https://www.microsoft.com/en-us/download/details.aspx?id=48145).

# Usage
## Editor
Get started by selecting one of the assembly examples through *File->Load Example->assembly*. In the **Editor** tab, you should now see that some RISC-V assembly code is present in the left view.
Try changing some values or adding more instructions - you should see that the right side will change accordingly. By default, this view shows the disassembled version of the binary instructions that the left view has been assembled into.
If you want to view the raw binary information, select *Binary* as *View mode*.

<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/asmeditorpng.png?raw=true"/>
</p>

## Processor
Once no syntax errors have been found in your assembly code, your code will automatically be assembled and loaded into the simulator.
Switching to the **Processor** tab, you'll be greeted by a datapath picturing a classic 5-stage RISC pipeline architecture. Note that a *Control* component nor most control signals aren't included in the view. This descision has been made to reduce clutter. All control signals are generated in the ID (Instruction decode) stage, and are propagated through the stage-separating registers accordingly.

<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/processortab.png?raw=true"/>
</p>

Besides the datapath, this tab contains the following elements:
* Simulator control:
    * **Step** is equal to activating the clock signal in the circuit. This will clock next-state signals into the registers, and new signals are calculated (propagate through the circuit)
    * **Autostepping** will automatically step through the program with a speed determined by the execution speed slider. 
    * **Run** will run the program until a breakpoint is encountered or execution is finished. This will postpone updating the visualized datapath until the break is encountered.
    * **Reset** resets the simulator, resetting all stages and setting the program counter to point to the first instruction
* Register view: A list of all registers in the processor, with highlighting for the most-recently used register
* Instruction memory: A view of the disassembled instructions that are currently loaded in the processor. This view displays which instructions are present in each pipeline stage, as well as breakpoint setting functionality
Furthermore, the following options are available:
* **Display all signal values** will show the underlying output signals for most signals in the datapath
* Various buttons are available for zooming & expanding the datapath view
* The datapath image can be saved to a file
* **Pipeline table** will display a pop-up window with the stage information for the current program. The program must be run to propagate the chart
<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/ptable.PNG?raw=true"/>
</p>

During execution, the instructions will propagate through the datapath, and be visible both above each stage of the pipeline, as well as in the instruction memory view. Furthermore, when the pipeline is stalled or flushed, this will be indicated above a pipeline stage.
## Memory
Selecting the **Memory** tab will show you three different views:
* **Registers** Is similar to the view presented in the processor tab, giving an overview of the contents of each register
* A memory view, where all parts of the program memory can be inspected. To navigate, one can use the scroll view or select one of the predefined destinations (or to a specific address) via the *Go to* combobox. A label for an address can be saved for future use, via. the *Save address* button. 
* A list of the most recent memory accesses. Selecting an entry and pressing *Go to selected address* sets the right side memory view to center at the address for the given entry

<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/memorytab.png?raw=true"/>
</p>

## Building
Since Ripes is built using pure C++ and Qt, all platforms that support Qt should be able to build and run the app.

### Minimal (Linux)
If you do not wish to download the entire Qt environment, the project can be built given the following dependencies are available
* *g++* version with support for C++14
* *qmake* with Qt version 5.5 or higher
A minimal Qt installation can be found at https://launchpad.net/~beineri.
A Qt environment can be set up as follows:
```
sudo add-apt-repository --yes ppa:beineri/opt-qt593-xenial
apt-get update -qq
sudo apt-get install qt59-meta-minimal
source /opt/qt59/bin/qt59-env.sh
```
Verify that you have the correct qmake version installed:
```
qmake --version
```
Navigate to the project folder and run
```
qmake Ripes.pro
make
```


### Using QtCreator (All platforms)
Download Qt for Open Source development at https://www.qt.io/download.
Using the Qt installer, only the prebuilt components for your chosen compiler needs to be selected. When installing Qt, you also install QtCreator. 

Open QtCreator and go to *Tools->Options->Build & Run->Kits* and make sure that your compilers have been detected properly and your kit (ie. your toolchain) is ready for use. A warning will be issued if you do not set a Debugger for your kit - this does not prevent you from building the project. 
Open *Ripes.pro* in QtCreator, and build in either release or debug mode.
When building Qt apps, various dynamic libraries are required. Running the app through QtCreator, all Qt dll's are loaded into the path, and available for the application without any extra effort. If you wish to deploy or distribute the application, the required dependencies must be packaged with the compiled binary.

---
Icons kindly provided by Flaticon authors: [Smashicons](https://www.flaticon.com/authors/smashicons), [Freepik](https://www.flaticon.com/authors/freepik), [Vectors Market](https://www.flaticon.com/authors/vectors-market) & [Pixel Buddha](https://www.flaticon.com/authors/pixel-buddha).

<a href="https://www.qt.io/">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/QtIcon.png" width="60" height="60" />
</a>
