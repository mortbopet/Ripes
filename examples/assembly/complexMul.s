# This example demonstrates an implementation of the multiplication of two
# complex numbers z = 1 + 3i, w = 5 + 4i.

.data
aa: .word 1 # Real part of z
bb: .word 3 # Imag. part of z
cc: .word 5 # Real part of w
dd: .word 4 # Imag part of w
str: .string " + i* "

.text
main:
    lw a0, aa
    lw a1, bb
    lw a2, cc
    lw a3, dd

    # Do complex multiplication of numbers a0-a3
    jal complexMul
    mv t0, a1   # Move imaginary value to t0
    mv a0, a0   # Move real value to a1

    # Print real value (in a0) by setting ecall argument to 1
    li a7, 1
    ecall

    # Print delimiter string (pointer in a0) by setting ecall argument to 4
    la a0, str
    li a7, 4
    ecall

    # Print imaginary value (in a0) by setting ecall argument to 1
    mv a0, t0   # Move imaginary value to a1
    li a7, 1
    ecall

    # Exit program
    li a7, 10
    ecall

myMult:
    li t0, 32   # Iteration variable
    li t3, 0    # initialize temporary product register to 0

  start:
    mv   t1, a1     # move multiplier to temporary register
    andi t1, t1, 1  # mask first bit
    beq  t1, x0, shift
    add  t3, t3, a0

  shift:
    slli a0, a0, 1
    srai a1, a1, 1  # make an arithmetic right shift for signed multiplication
    addi t0, t0, -1 # decrement loop index
    bnez t0, start  # branch if loop index is not 0
    mv   a0, t3     # move final product to result register
    jr x1

complexMul:
	# Place the 4 input arguments and return address on the stack
	addi sp, sp, -28
    sw x0, 24(sp) # tmp. res 2
    sw x0, 20(sp) # tmp. res 1
    sw ra, 16(sp) # return address
    sw a0, 12(sp) # a
    sw a1, 8(sp)  # b
    sw a2, 4(sp)  # c
    sw a3, 0(sp)  # d

    # (a + ib)(c + id) = (ac − bd) + i(ad + bc)
    # Step 1: a*c
    mv a1, a2   # Move C from a2 to a1
    jal myMult
    sw a0, 20(sp)   # push onto tmp. res 1

    # step 2: b*d
    lw  a0, 8(sp)
    lw  a1, 0(sp)
    jal myMult

    # step 3: (ac − bd)
    lw  t0, 20(sp) # Reload a*c from stack
    sub t2 t0 a0 # t2 contains real part of multiplication
	# push (ac − bd) onto tmp. res 1 from stack
    sw  t2, 20(sp)

    # Step 4: a*d
    lw  a0 12(sp)
    lw  a1 0(sp)
    jal myMult
    sw a0, 24(sp) # store a*d in tmp. res 2

    # step 5: b*c
    lw a0 8(sp)
    lw a1 4(sp)
    jal myMult
    mv a1 a0 # moving result to a1 saves us 1 operation later on

    # step 6: (ad + bc)
    lw  t0, 24(sp)  # Reload a*c from stack
    add a1, t0, a1  # a1 contains imag part of multiplication
    lw  a0, 20(sp)  # Load real result from tmp. res 1

    lw   ra, 16(sp) # Reload return address from stack
    addi sp, sp, 28 # Restore stack pointer
    jr x1
