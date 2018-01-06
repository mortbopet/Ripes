## RISC-V-SIM
RISC-V-SIM is a graphical 5-stage pipeline simulator for the RISC-V ISA.
RISC-V-SIM is written using Qt 5 and C++14.

## Downloading

## Usage
Get started by selecting one of the examples through *File->Load Example*. In the Processor tab, you can now:
* **Run**: Run the program until end of file or until a breakpoint is encountered (todo)
* **Start simulation**: Automatically step through the program with a stepping speed determined by the *Execution speed* slider. 
* **Step**: Manually step through the program

During execution, values for both internal registers and memory can be viewed in the Memory tab.


## Building
Download Qt for Open Source development at https://www.qt.io/download.
Using the online installer, you only need to select the prebuilt components for your chosen compiler. When installing Qt, your also install QtCreator. 
Open QtCreator and go to *Tools->Options->Build & Run->Kits* and make sure that your compilers have been detected properly. A warning will be issued if you do not set a Debugger for your kit - If you only wish to build the project, then this warning can be ignored. 
Open *RISC-V-sim.pro* in QtCreator, and build for either release or debug mode.
