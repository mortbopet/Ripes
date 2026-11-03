.text

#---------------------------------------------------------------
# Verifies that brk follows the Linux semantics (as relied upon by
# newlib's _sbrk): brk(0) returns the current program break, which
# initially is the (aligned) end of the program's data, and a
# successful brk returns the new program break.
#---------------------------------------------------------------

li sp, 0x7ffffff0

test_1: # brk(0) returns the initial break, just past .data
li gp, 1
li a0, 0
li a7, 214
ecall
mv s0, a0
la t0, DATA
addi t0, t0, 3 # end of .data
blt s0, t0, fail
andi t1, s0, 0xF # 16-byte aligned
bne t1, zero, fail
addi t0, t0, 16
bge s0, t0, fail

test_2: # Growing the heap returns the new break
li gp, 2
addi a0, s0, 64
li a7, 214
ecall
addi t0, s0, 64
bne a0, t0, fail

test_3: # brk(0) returns the updated break
li gp, 3
li a0, 0
li a7, 214
ecall
addi t0, s0, 64
bne a0, t0, fail

test_4: # A break overlapping the stack fails and returns the current break
li gp, 4
mv a0, sp
li a7, 214
ecall
addi t0, s0, 64
bne a0, t0, fail

test_5: # Shrinking the heap returns the new break
li gp, 5
mv a0, s0
li a7, 214
ecall
bne a0, s0, fail

success:
li a0, 42
li a7, 93
ecall
fail:
li a0, 0
li a7, 93
ecall

.data
DATA: .byte 1, 2, 3
