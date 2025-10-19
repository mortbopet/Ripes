.data
X: .word 0xBEEF
Y: .word -1

.text
main:

#------------------------------------------------------------------------------
# test for jump pipeline flushing
#------------------------------------------------------------------------------
test1_2_flush:
    addi gp, x0, 0
    nop

    j 1f
    nop

    # slot 1
    addi gp, x0, 1
    nop

    #slot 2
    addi gp, x0, 2
    nop

    1:
    bne gp, x0, fail
    nop

#------------------------------------------------------------------------------
# Test for correct return address calculation in jumps
#------------------------------------------------------------------------------
test3_return_adress:
    addi gp, x0, 3
    nop

    auipc x1, 0     # +0
    nop             # +4

    addi x1, x1, 24 # +8
    nop             # +12

    jal x2, 1f      # +16
    nop             # +20

    1:              # +24
    bne x1, x2, fail
    nop


#------------------------------------------------------------------------------
# test for simultaneous jumping and memory operation
#------------------------------------------------------------------------------
#  test for load  #
#-----------------#
test4_jmp_mem:
    # load address of X into x5
    lui x5, %hi(X)
    nop

    addi x1, x0, -1
    nop

    mv x2, x1
    nop

    # jump to label and simultaneously load X
    j 1f
    lw x2, %lo(X), x5   # x2 = *x5

    1:
        addi gp, x0, 4
        nop

        beq x2, x1, fail
        nop

#------------------#
#  test for store  #
#------------------#
test5_jmp_mem:
    # load high-address of X into x5
    lui x5, %hi(X)
    nop

    nop
    lw x1, %lo(X), x5   # x1 = *(&X)

    addi x5, x5, %lo(Y) # x5 = &Y
    nop

    # jump to label and simultaneously store X into Y
    j 1f
    sw x1, 0(x5)        # *(&Y) = X

    1:
        addi gp, x0, 5
        lw x2, 0(x5)    # x2 = *(&Y)

        # load delay slot
        nop
        nop

        bne x2, x1, fail
        nop



pass:
    li a0, 42

    li a7, 93

    j exit
    nop

fail:
    li a0, 0

    li a7, 93

    j exit
    nop

exit:
    # flush pipeline for ecall
    nop
    nop

    ecall
    nop