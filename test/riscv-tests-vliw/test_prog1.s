.data
X: .word 0xDEAD
Y: .word 0xBEEF

.bss
z:

.text

#------------------------------------------------------------------------------
# load and add X & Y into z and confirm value stored at z
#------------------------------------------------------------------------------

main:

lui t0, %hi(X)      # t0 = &X
nop

lui t1, %hi(Y)      # t1 = &Y
lw a0, %lo(X), t0   # a0 = *t0

lui t2, %hi(z)      # t2 = &z
lw a1, %lo(Y), t1   # a1 = *t1

addi t2, t2, %lo(z) # t2 = &z
nop

add a2, a0, a1
nop

nop
sw a2, 0(t2)        # *t2 = a0+a1

nop
lw a3, 0(t2)        # a3 = *t2


# test 1: equality of stored and loaded value
li gp, 1
nop

bne a3, a2, fail
nop

# test 2: correct value of calculation
# 0xDEAD + 0xBEEF = 0x19D9C = 105884
# => li a4, 0x19D9C, which assembles to
lui a4, 0x1a
nop

addi a4, a4, -612
nop

li gp, 2
nop

bne a3, a4, fail
nop

# all tests passed


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