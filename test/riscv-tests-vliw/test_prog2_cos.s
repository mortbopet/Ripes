
#------------------------------------------------------------------------------
# Test program which adds two vectors, calculates their dot product
# and verifies the results
# This program is compatible with single issue forwarding processors
# so that it can be co-simulated to test for integrity
#------------------------------------------------------------------------------

.equ SIZE 15

.data
X:  .word 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
Y:  .word 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0

Z:  .word 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
D:  .word 19840

.bss
a: .zero 4*SIZE

.text
main:
# s0 = X
# s1 = Y
# s2 = Z
# s3 = D
# s4 = a

# load *X
lui s0, %hi(X)
nop

addi s0, s0, %lo(X)
nop

# load *Y
lui s1, %hi(Y)
nop

addi s1, s1, %lo(Y)
nop

# load *Z
lui s2, %hi(Z)
nop

addi s2, s2, %lo(Z)
nop

# load *D
lui s3, %hi(D)
nop

addi s3, s3, %lo(D)
nop

# load *a
lui s4, %hi(a)
nop

addi s4, s4, %lo(a)
nop


# vec_add
mv a0, s0
nop

mv a1, s1
nop

mv a2, s4
nop

li a3, SIZE
nop

jal vec_add
nop


# test 1: vec_check
mv a0, s4
nop

mv a1, s2
nop

li a2, SIZE
nop

jal vec_check
nop

li gp, 1
nop

beq a0, x0, fail
nop


# test 2: vec_dot
mv a0, s0
nop

mv a1, s1
nop

li a2, SIZE
nop

jal vec_dot
lw s3, 0(s3)

li gp, 2
nop

bne a0, s3, fail
nop

# all passed
j pass
nop


# a2[0:a3] = a0[0:a3] + a1[0:a3]    # restoring a0, a1, a2, a3
vec_add:
    slli t2, a3, 2  # a4 = 4*a3
    nop

    1:
    addi a3, a3, -1
    lw t0, 0(a0)

    addi a0, a0, 4
    lw t1, 0(a1)

    addi a2, a2, 4
    nop

    add t0, t0, t1
    nop

    addi a1, a1, 4
    sw t0, -4(a2)

    bne a3, x0, 1b
    nop

    # reset arguments
    sub a0, a0, t2
    nop

    sub a1, a1, t2
    nop

    ret
    nop

# a0 = (a0[0:a2] == a1[0:a2])
vec_check:
    1:
    addi a2, a2, -1
    lw t0, 0(a0)

    addi a0, a0, 4
    lw t1, 0(a1)

    addi a1, a1, 4
    nop

    xor t2, t0, t1
    nop

    bne t2, x0, 2f  # t0 ^ t1 != 0  # ie t0 != t1
    nop

    bne a2, x0, 1b
    nop

    2:
    sltiu a0, t2, 1 # a0 = t2 <= 0
    nop

    ret
    nop

# a0 = dot(a0[0:a2], a1[0:a2])
vec_dot:
    li t2, 0
    nop

    1:
    addi a2, a2, -1
    lw t0, 0(a0)

    addi a0, a0, 4
    lw t1, 0(a1)

    addi a1, a1, 4
    nop

    mul t0, t0, t1
    nop

    add t2, t2, t0
    nop

    bne a2, x0, 1b
    nop

    mv a0, t2
    nop

    ret
    nop




pass:
    li a0, 42
    nop

    li a7, 93
    nop

    j exit
    nop

fail:
    li a0, 0
    nop

    li a7, 93
    nop

    j exit
    nop

exit:
    # flush pipeline for ecall
    nop
    nop

    ecall
    nop