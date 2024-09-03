.text

#-------------------------------------------------------------
# Stack-Heap Collision brk syscall Test
#-------------------------------------------------------------

test_0: # Test Ecall Return Value
    mv a0, sp # Assign the ecall parameter to the stack pointer value 
  	li a7, 214 # Load the brk syscall immediate value
    ecall

    # If the return value of the ecall is zero, then the ecall
    # was executed successfully and therefore the test failed
    beqz a0, fail

  bne x0, gp, pass
 
pass: 
 li a0, 42
 li a7, 93
 ecall


fail: 
 li a0, 0
 li a7, 93
 ecall