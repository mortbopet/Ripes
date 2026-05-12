.text

#---------------------------------------------------------------
# Program should write "BBAABBAABBAABBAABBAA" to /tmp/test.txt
# and then read back the file
#---------------------------------------------------------------

test_0: # Test open
la a0, FILENAME # a0: Pointer to filename
lw a1, O_RDWR # a1: Open flags
lw t0, O_TRUNC # Truncate file
or a1, a1, t0 # OR flags
lw t0, O_CREAT # Create file if it doesnt exist
or a1, a1, t0 # OR flags
lw a7, OPEN # a7: "Open" ecall
ecall # Returns: File descriptor to a0
mv s0, a0 # Save file descriptor in s0
li t0, 3 # File descriptor should be 3
bne a0, t0, fail

test_1: # Test write
lw s1, LOOPS # Number of times to duplicate strings
lw s2, BUF_START # Buffer to use for string duplication

L1:
la a1, AA # a1: Pointer to 'AA' string
andi t0, s1, 1 # Write 'AA' if even and 'BB' if odd
bne t0, zero, 1f # Skip 'BB' string if even
la a1, BB # a1: Pointer to 'BB' string
1:

lw t1, 0(a1) # Load 'AA' or 'BB'
sw t1, 0(s2), t0 # Store to buffer
lw a2, STRLEN # a2: Buffer size
add s2, s2, a2 # Next index in buffer

addi s1, s1, -1 # Subtract from loop counter
bne s1, zero, L1 # Write again

mv a0, s0 # a0: File descriptor
lw a1, BUF_START # a1: Buffer start
lw a2, STRLEN
lw s1, LOOPS
mul a2, a2, s1 # a2: Buffer size = (STRLEN * LOOPS)
mv s1, a2 # Bytes that should have been written
lw a7, WRITE # a7: "Write" ecall
ecall # Returns to a0: Number of bytes written
bne a0, s1, fail # Ensure write returned correct number of bytes

test_2: # Seek file to start
mv a0, s0 # a0: File descriptor
mv a1, zero # a1: Offset to seek
mv a2, zero # a2: Base of seek
lw a7, LSEEK # a7: LSeek syscall
ecall # Returns to a0: 0 if successful
bne a0, zero, fail

test_3: # Read from file
mv a0, s0 # a0: File descriptor
lw a1, BUF_START # a1: Buffer start
lw a2, STRLEN # a2: Number of bytes to read
lw t0, LOOPS # Number of times written
mul a2, a2, t0 # Multiply by number of times written
mv s1, a2 # Save number of bytes written to check if successful
lw a7, READ # a7: Read syscall
ecall # Returns to a0: (STRLEN * LOOPS) if successful
bne a0, s1, fail

j success

cleanup: # Close file
mv a0, s0 # a0: File descriptor
lw a7, CLOSE # a7: Close syscall
ecall
ret # Return to fail/success
fail:
jal cleanup
li a0, 0
li a7, 93
ecall
success:
jal cleanup
li a0, 42
li a7, 93
ecall

.data
.align 4

# Data to write to temp file
STRLEN: .word 2
AA: .string "AA"
BB: .string "BB"

# File name to use for this test
FILENAME: .string "./test.txt"

# Error messages
OPEN_FAILED: .string "Failed to open file"
WRITE_FAILED: .string "Failed to write to file"
READ_FAILED: .string "Failed to read from file"
SEEK_FAILED: .string "Failed to seek file to beginning"

# File open modes
O_RDONLY:  .word 0x0000
O_WRONLY:  .word 0x0001
O_RDWR:    .word 0x0002
O_ACCMODE: .word 0x0003
# Additional file flags
O_CREAT:  .word 0x0100
O_EXCL:   .word 0x0200
O_TRUNC:  .word 0x1000
O_APPEND: .word 0x2000

# Syscalls
PRINT_STR: .word 4
EXIT:  .word 10
CLOSE: .word 57
LSEEK: .word 62
READ:  .word 63
WRITE: .word 64
EXIT2: .word 93
OPEN:  .word 1024

# Other data
BUF_START: .word 0x2000
LOOPS: .word 10

