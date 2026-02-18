.data
X: .word 0xDEADBEEF

.text

#------------------------------------------------------------------------------
# This program will test the correct order of register access
# it is possible that both ways are reg-write-operation and both want to write
# into the same register. Obviously only one value can be stored.
# In our case this is predefined to be the data way value.
# Due to the forwarding and load delay slot this has interesting side effects,
# namely that the next following instructions in the pipeline will still
# receive the exec way value from the mem-stage forwarding. The 2nd following
# instruction will already receive the correct data value since in the wb stage
# the load operation is completed
#------------------------------------------------------------------------------

main:

lui x5, %hi(X)      # x5 = &X
nop

# both ways want to write to the same register in the same cycle
# however only one value can be stored, so we make the claim that
# the data way value will be stored
# => x1 = 0xdeadbeef (in register file)
addi x1, x0, -1     # x1 = -1
lw x1, %lo(X), x5   # x1 = *x5

addi x2, x1, 0      # forwarding x1 from mem,
nop                 # expect x1 = -1

addi x4, x1, 0      # forwarding x1 from wb,
nop                 # expect x1 = 0xdeadbeef

addi x6, x1, 0      # reading x1 in same cycle as its being written,
nop                 # expect x1 = 0xdeadbeef

addi x7, x1, 0      # reading x1 after instruction went through the pipeline,
nop                 # expect x1 = 0xdeadbeef


addi x30, x0, -1    # x30 = -1
lw x31, %lo(X), x5  # x31 = 0xdeadbeef

test2_reg_wr:
li gp, 2

bne x1, x31, fail   # assert x1 == 0xdeadbeef
nop

test3_fwd_mem:
li gp, 3

bne x2, x30, fail   # assert x2 == -1
nop


test4_fwd_wb:
li gp, 4

bne x4, x31, fail   # assert x4 == 0xdeadbeef
nop

test5_fwd_wb:
li gp, 5

bne x6, x31, fail   # assert x6 == 0xdeadbeef
nop

test6_fwd_wb:
li gp, 6

bne x7, x31, fail   # assert x7 == 0xdeadbeef
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