.text

#---------------------------------------------------------------
# Verifies that the file syscalls handle buffers at addresses with
# the most significant bit set, and that lseek works on write-only
# file descriptors.
#---------------------------------------------------------------

test_1: # Open write-only
li gp, 1
la a0, FILENAME
li a1, 0x1101 # O_WRONLY | O_CREAT | O_TRUNC
li a7, 1024
ecall
blt a0, zero, fail
mv s0, a0

test_2: # Write from a high address
li gp, 2
li s1, 0x80000100
li t0, 0xDEADBEEF
sw t0, 0(s1)
mv a0, s0
mv a1, s1
li a2, 4
li a7, 64
ecall
li t0, 4
bne a0, t0, fail

test_3: # lseek on a write-only fd
li gp, 3
mv a0, s0
li a1, 0
li a2, 1 # SEEK_CUR
li a7, 62
ecall
li t0, 4
bne a0, t0, fail
mv a0, s0
li a7, 57
ecall
bne a0, zero, fail

test_4: # Read into a high address
li gp, 4
la a0, FILENAME
li a1, 0 # O_RDONLY
li a7, 1024
ecall
blt a0, zero, fail
mv s0, a0
li s2, 0x90000000
mv a1, s2
li a2, 4
li a7, 63
ecall
li t0, 4
bne a0, t0, fail
lw t1, 0(s2)
li t0, 0xDEADBEEF
bne t1, t0, fail
mv a0, s0
li a7, 57
ecall
bne a0, zero, fail

test_5: # GetCWD into a high address
li gp, 5
li s2, 0x90000100
mv a0, s2
li a1, 4096
li a7, 17
ecall
ble a0, zero, fail
lbu t0, 0(s2)
beq t0, zero, fail

success:
li a0, 42
li a7, 93
ecall
fail:
li a0, 0
li a7, 93
ecall

.data
FILENAME: .string "./test_highaddr.bin"
