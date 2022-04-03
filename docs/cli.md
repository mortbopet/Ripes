# Command-line Interface

Ripes v.2.2.5 adds support for a command line interface. Through this, programs can be assembled/compiled and simulated on any of the available processor models.

An example execution could be:
```sh
./Ripes 
  --mode cli            \ # Enable command line interface
  --src foo.s           \ # Input file
  -t asm                \ # Input file type
  --proc "RV32_5S"      \ # Processor model
  --isaexts M,C         \ # ISA extensions to enable in the assembler/processor  
  --ipc                 \ # show IPC
  --cycles              \ # show # of cycles executed
  --pipeline              # Show pipeline state during execution
```

## Options

See `./Ripes --help` for further information.

| *Flag* | *Description* |
| ---- | ----------- |
|  --mode <mode>       |  Ripes mode Options: `(gui, cli)` |
|  --src <src>         |  Source file |
|  -t <type>           |  Source type. Options: `(c, asm, bin)` |
|  --proc <proc>       |  Processor model (see `./Ripes --help` for options). |
|  --isaexts <isaexts> |  ISA extensions to enable (comma separated). |
|  --timeout <timeout> |  Simulation timeout in milliseconds. If simulation does not finish within the specified time, it will be aborted. |
|  -v                  |  Verbose output and runtime status information. |
|  --output <output>   |  Report output file. If not set, report is printed to stdout. |
|  --json              |  JSON-formatted report. |
|  --all               |  Enable all report options. |
|  --cycles            |  Report cycles |
|  --iret              |  Report instructions retired |
|  --cpi               |  Report cycles per instruction (CPI) |
|  --ipc               |  Report instructions per cycle (IPC) |
|  --pipeline          |  Report pipeline state |
|  --regs              |  Report register values |
|  --runinfo           |  Report simulation information in output (processor configuration, input file, ...) |
|   --reginit <[rid:v]>|     Comma-separated list of register initialization values. The register value may be specified in signed, hex, or boolean notation. Format: `<register idx>=<value>,<register idx>=<value>` |


