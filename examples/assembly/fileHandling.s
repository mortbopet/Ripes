###########################################################
# This example shows how a file can be opened in create,
# read-write mode, content written into the file, lseek
# performed in file, reading from the file, using fstat
# to get the file info, and finally closing the file.
#
# Note: Check the current directory as ripes for the
# sample.txt file, also delete the file for the program
# to execute properly again.
###########################################################

.data
filename:       .string       "sample.txt"
content:        .string       "This is a sample text file. Hello ripes!"
newline:        .string       "\n"
display:        .string       "File size: "
fstat_struct:   .zero         128
readbuf:        .zero         64

.text
# Creating a file
la a0, filename
addi a1, zero, 0x242 # O_RDWR | O_CREAT | O_TRUNC
addi a7, zero, 1024
ecall

add t0, zero, a0 # file descriptor - t0

# Writing to the file
add a0, zero, t0
la a1, content
addi a2, zero, 41
addi a7, zero, 64
ecall

# LSeek to change the pointer to beginning
add a0, zero, t0
add a1, zero, zero
add a2, zero, zero
addi a7, zero, 62
ecall

# Reading from the file
add a0, zero, t0
la a1, readbuf
addi a2, zero, 41
addi a7, zero, 63
ecall

# Print the read content
la a0, readbuf
addi a7, zero, 4
ecall

# Closing the file
add a0, zero, t0
addi a7, zero, 57
ecall

# Fstat to get file information
add a0, zero, t0
la a1, fstat_struct
addi a7, zero, 80
ecall

# Printing the file size
la a0, newline
addi a7, zero, 4
ecall

la a0, display
addi a7, zero, 4
ecall

la a5, fstat_struct
lw a0, 48(a5) # As per Linux ABI offset 48 is file size
addi a7, zero, 1
ecall

# Note: As of 2025, the fstat syscall populates the 
# structure with just zeros, hence the file size 0.

# Standard exit from file
addi a7, zero, 10
ecall