.text

#---------------------------------------------------------------
# Verifies that the file syscalls are binary-safe: bytes which are
# not valid UTF-8 text are written and read back unmodified, fstat
# reports the file size, and read returns 0 at end of file.
#---------------------------------------------------------------

test_1: # Open
li gp, 1
la a0, FILENAME
li a1, 0x1102 # O_RDWR | O_CREAT | O_TRUNC
li a7, 1024
ecall
blt a0, zero, fail
mv s0, a0

test_2: # Write all bytes
li gp, 2
mv a0, s0
la a1, SRC
li a2, 8
li a7, 64
ecall
li t0, 8
bne a0, t0, fail

test_3: # Zero-length write
li gp, 3
mv a0, s0
la a1, SRC
li a2, 0
li a7, 64
ecall
bne a0, zero, fail

test_4: # fstat reports a regular file of 8 bytes
li gp, 4
mv a0, s0
la a1, STATBUF
li a7, 80
ecall
bne a0, zero, fail
la t1, STATBUF
lw t0, 16(t1) # st_mode
li t2, 0xF000 # S_IFMT
and t0, t0, t2
li t2, 0x8000 # S_IFREG
bne t0, t2, fail
lw t0, 48(t1) # st_size (low word)
li t2, 8
bne t0, t2, fail
lw t0, 52(t1) # st_size (high word)
bne t0, zero, fail

test_5: # fstat reports stdout as a character device
li gp, 5
li a0, 1
la a1, STATBUF
li a7, 80
ecall
bne a0, zero, fail
la t1, STATBUF
lw t0, 16(t1)
li t2, 0xF000
and t0, t0, t2
li t2, 0x2000 # S_IFCHR
bne t0, t2, fail

test_6: # lseek to the end returns the file size
li gp, 6
mv a0, s0
li a1, 0
li a2, 2 # SEEK_END
li a7, 62
ecall
li t0, 8
bne a0, t0, fail

test_7: # Rewind and read back more than was written
li gp, 7
mv a0, s0
li a1, 0
li a2, 0 # SEEK_SET
li a7, 62
ecall
bne a0, zero, fail
mv a0, s0
la a1, DST
li a2, 16
li a7, 63
ecall
li t0, 8
bne a0, t0, fail
la t1, SRC
la t2, DST
lw t3, 0(t1)
lw t4, 0(t2)
bne t3, t4, fail
lw t3, 4(t1)
lw t4, 4(t2)
bne t3, t4, fail

test_8: # Read at end of file returns 0
li gp, 8
mv a0, s0
la a1, DST
li a2, 16
li a7, 63
ecall
bne a0, zero, fail

test_9: # Close
li gp, 9
mv a0, s0
li a7, 57
ecall
bne a0, zero, fail

success:
li a0, 42
li a7, 93
ecall
fail:
li a0, 0
li a7, 93
ecall

.data
.align 4
SRC: .byte 0x00, 0xF0, 0x80, 0xFF, 0x0A, 0x0D, 0x1A, 0xC3
.align 4
DST: .word 0, 0, 0, 0
STATBUF: .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
FILENAME: .string "./test_binary.bin"
