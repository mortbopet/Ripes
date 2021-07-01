# This example demonstrates how strings, integers, chars and floating point
# values may be printed to the console

.data
str:        .string      "A string"
newline:    .string      "\n"
delimiter:  .string      ", "

.text
# ------ String printing ----------
    la a0, str # Load the address of the string, placed in the static data segment
    li a7, 4   # Argument '4' for ecall instructs ecall to print to console
    ecall

    jal printNewline

# ------ Integer printing ---------
# Print numbers in the range [-10:10]
    li a0, -10
    li a1, 10
    li a2, 1
    jal loopPrint

    jal printNewline

# -------- Float printing ----------
# Print an approximation of Pi (3.14159265359)
    li a0, 0x40490FDB
    li a7, 2
    ecall

    jal printNewline

# ------ ASCII character printing ---------
# Print ASCII characters in the range [33:53]
    li a0, 33
    li a1, 53
    li a2, 11
    jal loopPrint

    # Finish execution
    jal exit

# ====== Helper routines ======
printNewline:
    la a0, newline
    li a7, 4
    ecall
    jr x1

# --- LoopPrint ---
# Loops in the range [a0;a1] and prints the loop invariant to console
# a0: range start
# a1: range stop
# a2: print method (ecall argument)
loopPrint:
    mv t0 a0
    mv t1 a1
loop:
    # Print value in a0 as specified by argument a2
    mv a0 t0
    mv a7 a2
    ecall
    # Print a delimiter between the numbers
    li a7, 4
    la a0, delimiter
    ecall
    # Increment
    addi t0, t0, 1
    ble  t0, t1, loop
    jr x1

exit:
    # Exit program
    li a7, 10
    ecall
