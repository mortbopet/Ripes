# Building and Executing C programs with Ripes
Any processor model within Ripes which is *not* dependent on user-scheduled code is able to execute any executable compiled for the instruction set of which the processor model implements.  
The following sections walks through how a `.c` program may be compiled and subsequently executed in the simulator.

As an example (precompiled)##  program, refer to [ranpi.c](https://github.com/mortbopet/Ripes/blob/master/test/riscv-tests/ranpi.c). Note that a precompiled version of this program is bundled along with Ripes as one of the examples (within Ripes select `File->Load Example...->RanPI`).  
Notably, this program contains floating point operations whereas the simulated processor does not contain a floating point unit. Due to this (inferred by setting `-march=rv32im`, as seen below), the compiler will include [soft-float routines](https://gcc.gnu.org/onlinedocs/gccint/Soft-float-library-routines.html) implementing floating point operations through integer arithmetic.

## Toolchain
Initially, note that the RISC-V toolchain available through standard package repositories such as `gcc-7-riscv64-linux-gnu` will _not_ be able to produce files which are executable with the current version of Ripes, even if the `-march=rv32im` switch is used <sup>[1](#rv64tc)</sup>.

Ripes requires a bare-metal RISC-V toolchain, specifically the tuple`riscv64-unknown-elf-...`. This can be fetched from either:
* Download a prebuilt toolchain for your platform from `sifive/freedom-tools`. [This version is known to work well with Ripes](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only).
* Or, if you prefer to build your own toolchain, follow the instructions [https://github.com/riscv/riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain) for building the newlib toolchain.  
**Note**: the toolchain must be built with `--enable-multilib` to enable targeting 32-bit RISC-V systems.

## Toolchain Registration
<p align="center">
    <img src="https://github.com/mortbopet/Ripes/blob/master/resources/images/ccsettings.png?raw=true" />
</p>
With a valid toolchain available, navigate to `Edit->Settings->Editor`. Here, one may provide a path to a compatible compiler executable, ie. the `riscv64-unknown-elf-gcc` executable. Ripes will automatically try to search for the executable in the current `PATH`. If no compiler was found automatically, one may browse and navigate to the compiler executable. If a valid compiler has been specified, the `compiler path` field will turn green.  
One may specify additional compiler arguments in the `compiler arguments` field, such as optimization level etc.

**MacOS users** may experience that when trying to register the compiler, a popup may appear stating that ".. was blocked from use because it is not from an identified developer". This will appear for _all_ separate executables used by the toolchain (assembler, linker, etc..). 
This may be fixed by locating each of the executables mentioned in these popups (all programs are located within the toolchain parent folder) and manually validating through a `right click -> Open`.
Otherwise, navigate to `Security & Privacy -> General` and allow each of the programs used by the toolchain.  
Once all these programs have been allowed, toolchain registration should be successful.

## Compiling and Executing a C program
Navigate to the editor tab, and select `input type` to C (Note: an error will occur if no valid C compiler has been set). An example program could be:
```C
#include <stdio.h>

int main() {
   char buffer[128];
   printf("Type something to be echoed:\n");
   fflush(stdout);
   fgets(buffer, sizeof(buffer), stdin);
   printf("\nYou typed: %s", buffer);
   return 0;
}
```
Press the **Todo: insert image** button (or Ctrl+B) to build the program. If no syntax errors were found and the program was built successfully, the produced executable is automatically loaded into the simulator, and visible in the disassembled view to the right in the editor screen. Next, we may simulate the program as usual, by running or stepping through the program.

### Compiling Without the Standard Library
When building a C program with C standard library support, a lot of standard library support code gets linked into the executable in turn producing a program which is quite hard to navigate if one wishes to step through the program. In cases where no standard library is required, it is recommended to add the `-nostdlib` flag as a compiler argument. Then, _only_ the C functions written by the user will be linked into the produced executable.
When doing so, the entry point of the program will be the first instruction in the produced executable. As such, ensure that you manually place the entry point function to be the first function defined in your source code.

### Printing Without the Standard Library
When compiling with `-nostdlib` functions such as `printf` are unavailable. However, we may still wish to have support for printing to the console. In this case, it is recommended to implement some simple print function using the `ecall` functions (ecalls.md).
An example of a hello world program with `-nostdlib` enabled may be:
```c
void prints(volatile char* ptr);

void hello_world() {
    char* str = "Hello world";
    prints(str);
    return 0;
}

void prints(volatile char* ptr){ // ptr is passed through register a0
    asm("li a7, 4");
    asm("ecall");
}
```
Again, note that `hello_world` is placed as the first function in the program, to ensure that this is the entry point of the program.

# Manually Compiling a Program
The above sections describes how to automatically compile and load an executable program into Ripes via. the built-in compiler support. The following section describes how to manually run the compiler, and load a produced executable into Ripes.

## Manual Compilation and Execution
Navigate to the `bin` directory of where the newly built RISC-V toolchain was installed.
To produce a compatible executable, the following flags are considered:
- `-march=rv32im`: Target ISA (RV32I + RV32M).
- `-mabi=ilp32`: ABI specification<sup>[2](#rvtcconv)</sup>. int, long, pointers are 32-bit. GPRs and the stack are used for parameter passing.
- `-s`: Strip symbol information from the binary. _(optional)_

A program may then be compiled:
> `./riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -s  foo.c -o a.out`

To verify the ELF file we may read the ELF header of the program. Of importance, we expect the following to be true:
```
> readelf -h a.out
ELF Header:
  Class:                             ELF32
  Data:                              2's complement, little endian
  OS/ABI:                            UNIX - System V
  Type:                              EXEC (Executable file)
  Machine:                           RISC-V
  Flags:                             0x0
```
Based on the internal ISA specification of the currently loaded processor, Ripes will perform its own verification to determine whether an input ELF file is compatible.

## Executing
If all went well, your compiled executable should be successfully loaded through the 'load file' dialog within Ripes, and the program may be executed like any other Ripes simulation.

***
<a name="rv64tc">1</a>: It has been found that specifying `-march=rv32im` to the `gcc-7-riscv64-linux-gnu` toolchain will link to startup files containing instructions from the `C` ISA extension.

<a name="rvtcconv">2</a>: [RISC-V Toolchain Conventions](https://github.com/riscv/riscv-toolchain-conventions/blob/master/README.mkd)
