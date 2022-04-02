
## Adding a new Processor model

### 1. Write your model

Visual processor models are described in C++ using the [VSRTL](https://github.com/mortbopet/VSRTL) framework. At the end of this page, we also describe how to add a Verilog/SystemVerilog processor to Ripes (experimental).
The best way of getting started with how to describe your processor model in VSRTL is to take a look at existing implementation, e.g. the [single-cycle processor model](https://github.com/mortbopet/Ripes/blob/master/src/processors/RISC-V/rvss/rvss.h) or the (more complicated) [5 stage processor model](https://github.com/mortbopet/Ripes/blob/master/src/processors/RISC-V/rv5s/rv5s.h). While a bit verbose, the syntax of VSRTL should hopefully be easy to pick up for those familiar with existing HDLs.

### 2. Create a layout

The visualization of a processor model is described visually within a `json` file (e.g. the [standard layout for the single-cycle processor](https://github.com/mortbopet/Ripes/blob/master/src/processors/RISC-V/rvss/rv_ss_standard_layout.json)). In this file, each of the processor components is described (position, shape, size, rotation, ports, label, etc.). It also described the wires that connected the components.

The most convenient way to edit a layout file is within Ripes. In the Processor tab, right-click _outside_ of the processor itself and select `unlock`. You are now free to move around components, ports, and wires. Here is the most important info on component layout editing:

* To add a bend in a wire, simply right-click the wire and select `add wire point`. Wire points are invisible per default, and will only show up when you hover your mouse over them.
* To merge two (adjacent) wire points, select a point and drag it onto an adjacent point. This will also merge any wires connecting to the dragged point.
* to scale a component, drag in the lower-right hand corner of it.
* Labels can be moved by dragging them. Labels for a port can be enabled by right-clicking the port and selecting `show value/label/width`.
* Ports can be moved by selecting them. Hiding a port will hide any connecting wires. A hidden port will be shown once hovering your mouse over it, in case of wanting to re-enable it.
* Components can be hidden by right-clicking them and selecting `Hide component`. To show a hidden component, right-click _outside_ of the processor model and select `Hidden components -> ${component you want to show}`.

When you're done with editing the layout of your processor model, simply right-click _inside_ the processor model and select `Layout->Save layout`. This stores the `json` file, which can then be used internally in Ripes or as an input for `Layout->Load layout`.

*Note*: expanding/contracting components is somewhat buggy and does not yet restore layouts in-between expansions/contractions.

### 3. Add the model to Ripes

A directory specific to the new processor model must be created. It must contain the .h file describing the processor and the files describing the layout. To add a processor named new_proc, one must create the following directory and files:

- /src/processors/${ISA}/${model name}
  - ${model name}.h/cpp
  - ${model name}_layout.json

Most processor models in Ripes provide two different layouts, a _standard_ and _extended_ layout. The standard layout will hide components that may not be strictly necessary for a user to fully understand when they're just starting out with Ripes. The _extended_ layout will then contain _all_ relevant components of the design.

There are also a number of files where the new processor and its supporting files should be mentioned.
Here is the list of files where the new processor model has to be added: 
- [processorregistry.h](https://github.com/mortbopet/Ripes/blob/master/src/processorregistry.h)
- [processorregistry.cpp](https://github.com/mortbopet/Ripes/blob/master/src/processorregistry.cpp)
- [layouts.qrc](https://github.com/mortbopet/Ripes/blob/master/src/processors/layouts.qrc)
- [CMakeLists.txt](https://github.com/mortbopet/Ripes/blob/master/src/processors/CMakeLists.txt)

### 4. Verify
see *Debugging & Testing* below


## Debugging & Testing
Once a processor model has been implemented, it is expected to pass all of the accompanying unit tests bundled with Ripes.
To add a processor model, add an entry similar to i.e. `ProcessorID::RV5S` in the following files:
- [tst_riscv.cpp](https://github.com/mortbopet/Ripes/blob/master/test/tst_riscv.cpp#L67)
- [tst_cosimulate.cpp](https://github.com/mortbopet/Ripes/blob/master/test/tst_cosimulate.cpp#L79)

When verifying your design, it is strongly recommended to do this in conjunction with `tst_cosimulate.cpp`. When doing cosimulation, we run a test program on the single-cycle model, it being a reference model, and generate a trace of all of the register changes that occured during its execution of the test program. Then the test model is executed, wherein we compare the register changes to the reference change. If a divergence occurs, this indicates an error in the test model (due to the fact that the architectural state, as visible to software, must be equivalent between the two). In this, an indication of the program counter of each processor model, # of cycles as well as register differences, is indicated. Based on this info, one may run Ripes, navigate to the place in the program where the discrepancy occurred, and inspect the datapath to identify the error.

**_Note_**: Passing all unit tests is not a requirement for processor models which require software scheduled code (ie. models without forwarding etc..).

## (Experimental) Verilator Processor Models in Ripes

By using VSRTL to describe our processor models, we get added benefit of a visualization. However, VSRTL is not a fully-fledged HDL and users may find it difficult to express some constucts in it. Furthermore, our models may lack critical behaviours which are inherent to _real_ processor models.

An alternative is to use Verilator-generated models as a backend for Ripes. Through [Verilator](https://www.veripool.org/verilator/), we may take an existing verilog design and compile it into a C++ program. Assuming adequate signals are exposed by the processor, the processor model should be able to be integrated into the Ripes environment.

The benefits are clear - by inserting a processor model into the Ripes environment, a designer can use:
- the assembler, instruction visualizer, cache simulation capabilities of Ripes with their own model, while developing it
- ... Or execute a C program directly on your core!
- If tracing is enabled for the verilated model, this is also a very simple way to generate VCD files for further inspection/debugging

Preliminary work has been done to integrate the [PicoRV32](https://github.com/cliffordwolf/picorv32) core into Ripes. The work is available on [this](https://github.com/mortbopet/Ripes/tree/picorv32/src/processors/PicoRV32) branch. While a simple core, it demonstrates the process of using a Verilator-generated processor model in Ripes. Hopefully, this work can be a foundation/guide to include more complex multi-stage processors.

The following steps were taken to integrate the model into Ripes. This has **only** been tested on Linux (Ubunbtu 20.04).

- First, make sure that you have Verilator installed.
- The environment variable `VERILATOR_ROOT` must be set to base directory of verilator ([see this](https://veripool.org/guide/latest/install.html#installation)).
- The CMake variable `RIPES_BUILD_VERILATOR_PROCESSORS` must be set to enable the Verilated processor models.
- This information is used in the [top-level CMakeList.txt file to grab the Verilator CMake libraries](https://github.com/mortbopet/Ripes/blob/picorv32/CMakeLists.txt#L67)
- [In the given branch, the PicoRV32 project is added as a Git submodule](https://github.com/mortbopet/Ripes/tree/picorv32/src/processors/PicoRV32)
- In the processors subdirectory, the PicoRV32 model is enabled through CMake [here](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/CMakeLists.txt#L62). 
  - first we add the top-level file of PicoRV32 https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/CMakeLists.txt#L64. There is only a single file here, but it should be possible to add multiple files here, if required.
  - Next, we parameterize the PicoRV32 core. These parameters refer to the top-level generics defined in the verilog file: https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/CMakeLists.txt#L67-L72
  - Finally, we call [`create_verilator_processor`](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/CMakeLists.txt#L28). This macro creates a new library for the Verilator processor and adds it to the build. All non-verilog files are expected to be placed within a subdirectory named equally to the processor model. https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/CMakeLists.txt#L74-L80
- Next, we need to wrap the Verilator-generated files into a [`RipesProcessor`](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/interface/ripesprocessor.h) This is an interface which defines various hooks for the Ripes environment to interact with the processor model. Please read the documentation in `RipesProcessor` if in doubt about what each function does.
- In [`ripes_picorv32.h`](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.h) we define a translation between the states of the Verilog model and the corresponding state names (https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.h#L17-L32). These names will be used in the program viewer when stepping through the program. The states and state names themselves have been generated through inspection of the PicoRV32 source files.
- In [`ripes_picorv32.cpp`](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.cpp) we define the actual interaction with the Verilated model. 
  - First, [we include the verilated model](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.cpp#L4). This header will be placed in the output build directory when CMake is run.
  - Next, [we define what happens when the processor is clocked](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.cpp#L20).
    - [We first handle any memory accesses](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.cpp#L90). Here, we inspect the top-level signals of the processor and use these to read/write from the simulator memory.
    - [Next, we clock the processor flipping the `clk` wire and evaluating the circuit after doing so.](https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/PicoRV32/ripes_picorv32.cpp#L26-L29)
    - [Next, we handle any PCPI accesses, such as ECALL instructions](https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/PicoRV32/ripes_picorv32.cpp#L30-L39). The `m_doPCPI` variable is used to synchronize with respect to how the PicoRV32 processor expects the external environment to statefully handle PCPI requests. [`traphandler`](https://github.com/mortbopet/Ripes/blob/picorv32/src/processors/PicoRV32/ripes_picorv32.cpp#L37) is called, which will transfer control to the Ripes environment. Ripes will then expect the various `a0-a7` registers to determine the `ecall` that is being executed.
  - The [rest of the functions in the file](https://github.com/mortbopet/Ripes/blob/c8361a9e2bf56ff4591787b2b9393aaf5d99de90/src/processors/PicoRV32/ripes_picorv32.cpp#L118-L148) considers themselves with extracting signals/registers within the design to fulfil the `RipesProcessor` interface.
- [the processor is added to the processor registry](https://github.com/mortbopet/Ripes/blob/picorv32/src/processorregistry.cpp#L113) so we can instantiate it from within the GUI.
- [The processor is added to the cosimulation tests](https://github.com/mortbopet/Ripes/blob/picorv32/test/tst_cosimulate.cpp#L85). This is a good indicator that everything is working successfully, since we're executing large C programs through there.

**Limits:**
- I found that Verilator does not want to verilate a component in a `.v` file which does not look like a top-level component. Due to this, i had to make my own fork of the PicoRV32 repo and move unwanted top files from `picorv32.v` (see https://github.com/mortbopet/picorv32/commit/a102d011e16c19dfd34a8b640e6d30c80cf4d7d1)
- Ecall/Ebreak handling in PicoRV32 didn't play well initially with how we wanted to handle these wrt. interacting with Ripes. A fix was to parameterize as follows (https://github.com/mortbopet/picorv32/commit/338b33a8e12ac516e6b59296b336b5c5830fa569)
