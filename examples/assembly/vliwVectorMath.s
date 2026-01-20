#------------------------------------------------------------------------------
# VLIW - Very Long Instruction Word
#------------------------------------------------------------------------------
# Example Program:     Linear matrix computation (A * x + b)
# Required Extensions: M
# Compatibility:       5S-VLIW (only)
#------------------------------------------------------------------------------
# This example demonstrates the effective use of the statically scheduled,
# dual-issue VLIW processor pipeline, as described in the processor model.
# It highlights key aspects of the instruction-level parallelism and pipeline
# behavior using a practical linear algebra workload.
#
# NOTE:
#   This program is specifically written for the implemented 5S-VLIW processor
#   and is NOT compatible with conventional processor architectures that do not
#   implement the same VLIW execution model and instruction layout.
#------------------------------------------------------------------------------

# Data type configuration
# .byte  = log2(1) = 0
# .half  = log2(2) = 1
# .word  = log2(4) = 2
# .dword = log2(8) = 3
.equ SIZE_T      4        # Size in bytes of a vector element
.equ LOG_SIZE_T  2        # log2(size in bytes of vector element)
.equ SIZE        4        # N: dimensionality of vectors

#------------------------------------------------------------------------------
# Data Section
#------------------------------------------------------------------------------
.data

# 4x4 Matrix (A)
# Row-major layout:
#     00 01 02 03
#     10 11 12 13
#     20 21 22 23
#     30 31 32 33
A:  .word 00, 01, 02, 03,  10, 11, 12, 13,  20, 21, 22, 23,  30, 31, 32, 33

# 4x1 Input Vectors
x:  .word   01, 02, 03, 04
b:  .word   51, 52, 53, 54

# Output Labels
str_Ax:  .string "A * x = "
str_Axb: .string "A * x + b = "

#------------------------------------------------------------------------------
# Code Section
#------------------------------------------------------------------------------
.text

main:

la s0, A

la s1, x

la s2, b


# mat-vec mul ---------------
# t = A * x

mv a0, s0               # load argument A
nop

mv a1, s1               # load argument x
nop

li a2, SIZE             # load argument N

jal vec_mul
nop

mv s3, a0               # save result vector pointer
nop

# print string "A * x = "
la a0, str_Ax

jal print_string
nop

# print result vector t
mv a0, s3
nop

li a1, SIZE             # load argument N

jal vec_print
nop


# vec-add -------------------
# t = t + b
mv a0, s3               # load argument t = A * x
nop

mv a1, s2               # load argument b
nop

li a2, SIZE             # load argument N

jal vec_add
nop

mv s3, a0
nop


# print string "A * x + b = "
la a0, str_Axb

jal print_string
nop

# print resulting vector
mv a0, s3               # load argument t
nop

li a1, SIZE             # load argument N

jal vec_print
nop


# jump to end
j exit
nop


#------------------------------------------------------------------------------
# Function: vec_mul
# Purpose:  Compute right-sided matrix-vector multiplication
# Arguments:
#   a0 -> pointer to matrix A (NxN)
#   a1 -> pointer to vector x (Nx1)
#   a2 -> size N
# Returns:
#   a0 -> pointer to resulting vector (Nx1)
#------------------------------------------------------------------------------
vec_mul:
    slli t0, a2, LOG_SIZE_T # to size in bytes
    nop

    sub sp, sp, t0      # reserve size for returning vector
    nop

    # Save caller context and arguments
    addi sp, sp, -20    # reserve size for function arguments
    sw a0, -4(sp)       # 16(sp) a0

    nop
    sw a1, 12(sp)       # 12(sp) a1

    nop
    sw a2,  8(sp)       #  8(sp) a2

    nop
    sw s0,  4(sp)       #  4(sp) s0

    nop
    sw ra,  0(sp)       #  0(sp) return address

    li s0,  0           # loop counter

    # loop body
    1:
    jal vec_dot         # compute vector dot product of A_row * x
    nop
    
    slli t0, s0, LOG_SIZE_T # s0 to size in bytes
    nop

    add t0, t0, sp      # get a shifted stack pointer for return vector indexing
    lw a2,  8(sp)       # load size

    nop
    sw a0, 20(t0)       # store row dot product
    
    addi s0, s0, 1      # increment loop counter
    lw a0, 16(sp)       # load last matrix row pointer

    slli t0, a2, LOG_SIZE_T # step size to byte size
    lw a1, 12(sp)       # load vector pointer

    add a0, a0, t0      # increment matrix row pointer
    nop

    blt s0, a2, 1b      # loop if s0 < N
    sw a0, 16(sp)       # and store incremented matrix row pointer

    
    # end of matrix-vector multiplication
    # restore stack and return
    addi sp, sp, 20     # restore stack pointer
    lw ra,  0(sp)       # restore return address

    mv a0, sp           # save result pointer
    lw s0, -16(sp)      # restore s0

    ret                 # return from function
    nop


#------------------------------------------------------------------------------
# Function: vec_add
# Purpose:  Add two vectors element-wise
# Arguments:
#   a0 -> pointer to vector 1 (Nx1)
#   a1 -> pointer to vector 2 (Nx1)
#   a2 -> size N
# Returns:
#   a0 -> pointer to resulting vector (Nx1)
#------------------------------------------------------------------------------
vec_add:
    slli t0, a2, LOG_SIZE_T # convert N to bytes size
    nop

    sub sp, sp, t0      # reserve size for returning vector
    nop

    # loop body
    1:
    addi a0, a0, SIZE_T # increment vector pointer a0 and
    lw t1, 0(a0)        # load element from vector a0

    addi a1, a1, SIZE_T # increment vector pointer a1 and
    lw t2, 0(a1)        # load element from vector a1

    addi a2, a2, -1     # decrement loop counter
    nop

    add t1, t1, t2      # element-wise addition
    nop

    addi sp, sp, SIZE_T # increment stack pointer index
    sw t1, 0(sp)        # and store addition in result vector

    bne a2, x0, 1b      # loop if elements remain
    nop

    # end of vector addition
    # restore stack and return
    sub sp, sp, t0      # reset stack pointer to point again at vector
    nop

    mv a0, sp           # save result pointer
    nop

    ret
    nop


#------------------------------------------------------------------------------
# Function: vec_dot
# Purpose:  Compute dot product of two vectors
# Arguments:
#   a0 -> pointer to vector 1 (Nx1)
#   a1 -> pointer to vector 2 (Nx1)
#   a2 -> size N
# Returns:
#   a0 -> scalar dot product result
#------------------------------------------------------------------------------
vec_dot:
    li t2, 0            # initialize accumulator

    # loop body
    1:
    addi a0, a0, SIZE_T # increment vector a0 pointer
    lw t0, 0(a0)        # load vector a0 element

    addi a1, a1, SIZE_T # increment vector a1 pointer
    lw t1, 0(a1)        # load vector a1 element

    addi a2, a2, -1     # decrement loop index
    nop

    mul t0, t0, t1      # multiply elements
    nop

    add t2, t2, t0      # accumulate sum
    nop

    bne a2, x0, 1b      # loop if elements remaining
    nop

    # end of vector dot product
    mv a0, t2           # move result int0 a0
    nop

    ret
    nop


#------------------------------------------------------------------------------
# Function: print_string
# Purpose:  Print a null-terminated string to the console
# Arguments:
#   a0 -> pointer to string
#------------------------------------------------------------------------------
print_string:
    li a7, 4            # printString ecall id

    # flush EX pipeline stage
    nop
    nop

    # flush MEM pipeline stage
    nop
    nop

    # flush WB pipeline stage
    nop
    nop

    ecall
    nop
    
    ret
    nop


#------------------------------------------------------------------------------
# Function: vec_print
# Purpose:  Print vector contents in formatted form
# Arguments:
#   a0 -> pointer to vector (Nx1)
#   a1 -> size N
#------------------------------------------------------------------------------
vec_print:
    addi sp, sp, -8     # reserve stack for strings
    sw ra, -4(sp)       # save return address

    # preamble
    addi t1, x0, 0x5B   # opening bracket
    sw x0, 0(sp)        # fill string with null terminators

    addi t2, x0, 0x20   # ' '
    sb t1, 0(sp)

    mv t0, a0           # save vector ptr
    sb t2, 1(sp)

    mv a0, sp
    nop

    jal print_string
    nop

    addi t1, x0, 0x2C   # ','
    nop

    nop
    sb t1, 0(sp)


    # loop
    1:
    addi a7, x0, 1      # printInt ecall id
    lw a0, 0(t0)        # load vector element

    addi a1, a1, -1     # decrement loop counter
    nop

    addi t0, t0, SIZE_T # increment vector pointer
    nop

    mv a0, sp           # pipeline load of sp for print_string call after ecall
    nop

    ecall               # print integer vector element
    nop

    beq a1, x0, 2f
    nop

    jal print_string    # print element separator ", "
    nop

    j 1b
    nop


    # epilogue
    2:
    addi t0, x0, 0x20   # ' '
    nop

    addi t1, x0, 0x5D   # closing bracket
    sb t0, 0(sp)

    addi t0, x0, 0x0A   # line feed
    sb t1, 1(sp)
    
    mv a0, sp
    sb t0, 2(sp)

    addi sp, sp, 8      # decrement stack pointer
    nop

    # since print_string does not use the stack we can
    # prematurely decrement decrement the stack pointer
    # and let print_string return to the callee of vec_print
    j print_string      # tail call print_string
    lw ra, -4(sp)       # and restore return address


#------------------------------------------------------------------------------
# Exit Program
#------------------------------------------------------------------------------
exit:
    li a7, 10 # exit code

    # flush EX pipeline stage
    nop
    nop

    # flush MEM pipeline stage
    nop
    nop

    # flush WB pipeline stage
    nop
    nop

    ecall
    nop