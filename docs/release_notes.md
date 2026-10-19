# Ripes Release Notes

## Ripes v2.3.0

### New features
- **Qt 6**: Ripes has been migrated to Qt 6, and the macOS release is now a universal (Apple Silicon + Intel) build.
- **Ripes in the browser**: a WebAssembly build of Ripes is available at [ripes.dk](https://ripes.dk).
- **New processor models**:
  - A 5-stage multicycle processor (with one or two memories), based on the work of Francisco Valero Gil.
  - An ISA simulator "processor" model (`RV32_ISS`/`RV64_ISS`) for fast, non-pipelined execution.
- **Theming**: Light/Dark/System color schemes with live theme switching.
- **New memory-mapped I/O peripherals**: a raster screen, a real-time clock, and an edge-capture/latching D-pad.
- **Command line mode**:
  - ELF and C files can now be executed from the CLI.
  - System input (stdin) handling for CLI execution.
  - Cache simulation is now available in the CLI (`--cache-preset`).
- **Assembler**:
  - GNU-style `offset(base)` memory operands (e.g. `lw t0, 4(sp)`, `lw t0, (sp)`, `jalr ra, 4(t0)`, `c.lw a0, 4(a1)`, `c.lwsp a0, 4(sp)`).
  - Labels may now start with a dot.
- **Caches**: non-power-of-two associativities are now supported, and cache configuration now uses the term "sets" instead of "lines".
- **UI**:
  - Added a memory viewer widget to the edit and processor tabs; register values can be copied from the register widget and followed into memory (right click).
  - Panning in the cache graphic view, a modernized sidebar, and persisted window geometry.
  - Added a setting for enabling VSRTL VCD tracing.

### Bug fixes
- **Processor models**:
  - `jalr` did not clear the least significant bit of the target address (#477).
  - Register shift instructions (`sll`/`srl`/`sra`) used all bits of `rs2` instead of only the lower 5 (RV32) or 6 (RV64) bits (#398).
  - Incorrect `mulh`/`mulhu`/`mulhsu` results on 64-bit processors.
  - Compressed `c.j`/`c.jal` instructions jumping to the wrong address for offsets below -1024 (#472).
  - Incorrect EX stage stall in the 5-stage pipeline (#444).
  - Fixed RVC prime register encoding/decoding.
- **Assembler**:
  - `c.j`/`c.jal`/`c.beqz`/`c.bnez` encoded label targets as absolute rather than PC-relative addresses (#411).
  - `sw rs2, SYMBOL(rs1)` with a symbolic offset was assembled as the `sw rs2, label, rt` pseudo-instruction (#467).
  - Superfluous instruction operands are now rejected rather than silently ignored.
  - Disabled ISA extensions of a processor were still accepted by the assembler.
  - Hex/binary representations of edge-case signed immediates (e.g. `0xFFFFFFFF`) are now accepted.
  - Division by zero in constant expressions crashed the assembler (#347).
  - Parentheses were matched before comments were removed (#421).
- **Caches**: fixed the random replacement policy, ensured a minimum of one way, and fixed numeric limits for way counts and presets.
- **System calls**: fixed the `read` syscall character count being halved, the `O_CREAT` flag in `open`, read-only file operations, and added a stack-heap collision check to `brk`.
- **UI and misc**:
  - Auto-clocking was not stopped when a breakpoint was hit.
  - A breakpoint at line 0 was also mirrored at line `num instructions`.
  - I/O devices are now updated upon processor reset.
  - Fixed a crash when hovering over the cache statistics chart.
  - Fixed memory leaks in I/O simulation.
  - Console input and clipboard usage in the WebAssembly build (#304).
  - Source code highlighting with programs loaded at a non-zero start address.

## Ripes v2.2.6

### Bug fixes and minor stuff
- Changed the terminology for caches, substituting "Word" for "Block". Most textbooks use the term "cache block" interchangeably with "cache line", so using "Block" for words stored in a cache line was confusing.

## Ripes v2.2.5

### New Features:
- Ripes command line mode
  - Ripes can now be executed without the GUI to support batch and headless execution, as well as to integrate into other flows.
  - An example run is: `./Ripes --mode sh --src complexMul.s -t asm --proc "RV32_6S_DUAL" --isaexts M,C --cpi --cycles --pipeline`
  - See `./Ripes --help` for the full list of supported telemetry and options.

### Bug fixes and minor stuff
- Added new example for reading from STDIN in assembly
- Fixed deadlock when resetting processor while waiting for user input
- Fixed zombie state when exiting the application while "running" the processor
- Fixed string literal tokenizer; '\\"' and ':' are now allowed inside string directives.
- Added register column to system call help window.
- Added support for local/relative labels in assembler.
- Fixed pipeline diagram not updating

## Ripes v2.2.4

### New features:
- Support for the RISC-V compressed instruction set for all processors (big thanks to @lcgamboa for taking care of the processor model implementation!).
- Support for clang-format'ting `.c` files
- Added source code stage highlighting. Enabled for C code when programs are compiled with `-g`.  
**Note**: If you've used Ripes prior to this version, you must manually add `-g` to "compiler arguments in `settings->compiler->compiler arguments`. For new users, `-g` will be added by default.

### Bug fixes:
- Fixed race condition when autoclocking the single cycle processor which allowed for executing instructions beyond an `ecall exit` instruction
- Fixed issue in verifying ELF flags, which could sometimes lead to compiler autodetection failing
- Fixed issue in reloading integer settings values which represented unsigned integers
- A bunch of small fixes to issues relating to persisting save file path information.
- Fixed issue in loading flat binary files containing `0x0D` (CR) characters.
- Fix issue where parent directories were not being created when saving files.

## Ripes V2.2.3

### Bug fixes:
- Memory viewer showing incorrect values for 64-bit processors
- Fix some 32-bit hardcoding in cache sim and cache viz
- ProcessorHandler futurewatcher signals were reconnected on each invocation of `run`, essentially creating a linear increase in execution time after each `run`.

## Ripes v2.2.2

### New features/changes
- 64 bit support (RV64IM) for all current processors. This implies support for RV64IM instructions in the built-in assembler as well as compiler support for RV64 compatible baremetal toolchains
- Added a register view in the edit tab (may be disabled in settings)
- The current processor and ISA are now shown in the status bar
- Added `%hi` and `%lo` relocations
- Cache plot marker values are now shown in cache legend
- Various quality of life changes in the editor (indentation, parenthesis completion, ...)

### Bug fixes:
- Stage information was not always being update in the program viewer (only an issue with the 6 stage model).
- The symbol navigator was using incorrect symbol addresses. This was only a real issue on large, compiled programs.
- Fixed layering issues with port value labels
- Fix crashes on startup and shutdown on macOS
- Fixed cache graphic not updating after run finished
- Fixed possible crash when changing cache preset during stepping the processor

## Ripes v2.2.1
Bug fixes:
- Fix assembler error on parenthesis inside string directives #91
- Immediate values inside string directives were being highlighted separately, and not as part of the string
- Fix possible overwrite of previously saved program when loading a new example
- Fix possible invalid iterator dereference in ProgramViewer
- Fix (in some cases?)/improve compiler execution on Windows and elaborate compiler error message
- Change default compiler argument from `static-libgcc` to `-static`

## Ripes v2.2.0

### New features
- Complete rework of the assembler/disassembler infrastructure:
  - Instructions taking immediate values now support simple expression evaluation. Furthermore, any label may be used as an immediate value.
  - New infrastructure significantly simplifies adding new ISAs and ISA extensions.
  - Added `.equ` directive, for defining constants in the assembler
- ISA extensions can now be toggled for a given processor. ISA extension changes are reflected in the assembler, syntax highlighter, and C compiler `--march` strings.
- A 6-stage dual-issue processor model is now available. This is the first processor model in Ripes capable of reaching an IPC > 1.
- A 5-stage with hazard detection but no forwarding is now available.
- Various toolbuttons have been moved to clean up the UI.
- Various performance improvements to remove unnecessary model and UI updates.
- Modified cache simulator:
  - The cache simulator now has a dedicated tab.
  - The cache view now visualizes how the block and line indices map into the cache.
  - It is possible to save and load user cache presets
  - More focus on cache statistics, which includes real-time plotting of cache variable ratios as well as plotting of the moving average of a given ratio.
- A new tab has been added, the 'memory' tab. Here, the scrollable memory viewer is placed, alongside a new view of the memory map of the processor (segments of the loaded program and I/O devices).
- Support for memory mapped I/O is now available. Please refer to the [docs page on memory-mapped I/O](mmio.md) for more information.

### Bug fixes:
- Fixes issue with compiler paths containing " " (space) characters not being recognized as valid. 
- System call function register is now explicitly stated when showing system call error messages, as well as in the system call help menu.
- The new assembler infrastructure fixes quite a few bugs in the old assembler.

### VSRTL:
- Added support for darkmode-style circuit drawing
- Removed an unneeded dynamic_cast which resulted in a large speedup, due to the elimination of a vtable lookup for each port in the design, for each cycle.
- Remove use of templated `value<T>` function in favor of `uValue/sValue` which is bitwidth aware.

## Ripes v2.1.0

### New features
- **Cache simulation**
  - As of version 2.1.0, Ripes includes cache simulation. The cache simulator simulates L1D (data) and L1I (instruction) caches, wherein it is possible to configure the geometry and behavior of each cache type. Similarly to all of the included processor models, it is possible to rewind (undo a clock cycle) the state of the cache simulator to investigate any situation of interest.
  - Refer to [this docs page](cache_sim.md) for more info on how to use the cache simulator.
- **Compiler integration**
  - It is now possible to write and execute C-language programs right within Ripes. To do so, a compatible compiler must be downloaded and registered with the program. After this, C programs may be written and simulated, with support for most of the C standard library.
  - Refer to [this docs page](c_programming.c) for more info on how to build and execute C language programs within Ripes.
- A new **symbol navigator** window has been added to the edit tab. Pressing the compass icon in the toolbar of the edit tab will bring up a list of all symbols in the current program. Through this, it is possible to navigate the program view to any of these symbols. This features is useful when loading large programs with many non user-written symbols, for instance when compiling programs using the C standard library.
- Added dialog displaying all supported system calls based on the currently selected processor (ISA/ABI). Located at `Help->System calls`.
- Added precompiled version of [ranpi.c](https://github.com/mortbopet/Ripes/blob/master/test/riscv-tests/ranpi.c) as a bundled example. With this, users now have easy access to a benchmark-like program for investigating the behavior of a larger program.
- Added C examples
- Revamped built-in console to allow for user input, if required by an executing system call.
- Various settings have been added under a new settings window located under `Edit->Settings`.

### Bug fixes
- The memory viewer in the memory tab is now included in a splitter widget, along with the new cache widget. This fixes an issue when running Ripes on smaller screens, wherein the address column of the memory view was being compressed to a point where the address was not visible. With the splitter, the memory view can now be resized.
- Selecting "New program" while an ELF file was loaded would not automatically re-enable the assembly editor.
- Disassembling AUIPC and LUI instructions showed immediate values as being in base 10 rather than base 16.

#### VSRTL:
- Signal value labels are now updated once "running" the processor is finished/cancelled.

## Ripes v2.0.1 

### New features
- Disassemble view has been reworked to align with a typical `objdump` output, showing labels, instruction addresses etc.
- Disassembly view now highlights the currently executing instructions for each stage

### Bug fixes
- Fix misaligned PC4 output ports for MEMWB registers in extended layouts for RV5S and RV5S_NO_HZ
- Fix possible crash when using load dialog after having loaded an ELF file
- Target address when disassembling `jal` instructions of a program loaded at a non-zero start address was miscalculated.

#### VSRTL:
- Tooltips are no longer shown when hovering over a hidden wire
- Hovering over a port value label will display the tooltip of the associated port

## Ripes v2.0.0
The latest major release of Ripes represents a major overhaul of most parts of the codebase; mainly the simulator/visualization infrastructure as well as most of the MVC models used throughout.  
As of version 2.0.0, Ripes is now based on simulators implemented in [VSRTL](https://github.com/mortbopet/VSRTL), a C++ framework for describing and visualizing digital circuits.  
For an introduction to many of the new features, please refer to the [introduction](introduction.md) docs page.

### What's new
#### Multiple Processor Models
The set of processor models shipping in version 2.0.0 (described below) aims to address each level of added complexity when going from a single cycle processor to a fully functioning, in-order pipelined processor. Version 2.0.0 introduces the following processor models:

   - Single cycle
   - 5 Stage without hazard detection/resolution and fowarding
   - 5 Stage without hazard detection/resolution
   - 5-stage Pipeline

Each processor is provided with two layouts:
- **Standard**: A simplified view of the processor. Control components and signals are omitted.
- **Extended**: An extended view of the processor. Control components and signals are visible as well as wire bit-widths.

#### Reversible Circuits
Utilizing VSRTLs reversible circuit simulation, it is now possible to "undo" clocking the circuit (denoted as *reversing*). This may come in handy for investigating specific pipeline situations and how they were set up.  
All UI parts, such as the instruction view and execution statistics are modified accordingly, when reversing the circuit.

#### Interactible Processor View
- Clicking a port of a component will highlight the wire connected to the given port. This is especially helpful when trying to get an overview of some of the more complex circuit layouts.
- If a different arrangement of the components is desired, the view may be unlocked (right click outside the processor model). Then, components may be moved and wires may be modified.

#### ELF support
Ripes may now load and execute ELF files compiled for the RV32IM instruction set. For a guide on how to compile and execute an example C program, refer to the following docs page:
[Building and Executing C Programs with Ripes](c_programming.md)

#### UI/Usage Changes
- The `ecall` function code is now required to be in register `a7`, compatible with the linux ABI as well as other popular educational simulators.
- (Almost) all parts of the UI have been rewritten, removing a bunch of ugly code, hacks and bugs.
