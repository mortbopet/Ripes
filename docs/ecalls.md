## Supported Environment Calls
### Version 2.1.0 and later
All supported environment calls for your version of Ripes is described in the `Help-> System calls` menu, within the application.

### Version 2.0.0
Ripes supports the following environment calls:

|  `a7`  |          `a0`         |     *Name*     |                          *Description*                         |
|:--:|:-------------------:|:------------:|:------------------------------------------------------------:|
| 1  | (integer to print)  | print_int    | Prints the value located in `a0` as a signed integer         |
| 2  | (float to print) | print_float | Prints the value located in `a0` as a floating point number | 
| 4  | (pointer to string) | print_string | Prints the null-terminated string located at address in `a0` |
| 10 | -                   | exit         | Halts the simulator|
| 11 | (char to print) | print_char | Prints the value located in `a0` as an ASCII character |
| 34 | (integer to print) | print_hex | Prints the value located in `a0` as a hex number |
| 35 | (integer to print) | print_bin | Prints the value located in `a0` as a binary number |
| 36 | (integer to print) | print_unsigned | Prints the value located in `a0` as an unsigned integer |
| 93 | (status code) | exit | Halts the simulator and exits with status code in `a0` |

## Example Usage
### Printing to console
```
.data
str: .string "abc"

.text
li a0 42
li a7 1
ecall     # prints "42" to console

li a7 11
ecall     # prints "*" to console (ASCII(42) = '*')

la a0 str
li a7 4
ecall     # Prints "abc" to console
```

### Stopping the simulator
To stop the Ripes simulator, either use the `ecall` convention as below, or branch to a label that is located at the end of the `.text` segment in the source code.
```
.text
main:
li a7 10
ecall
# The following instructions will not get executed
li a1 1
li a2 1
li a3 1
```
