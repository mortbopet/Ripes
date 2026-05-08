.text
.globl _start
_start: nop

  #-------------------------------------------------------------
  # MULH / MULHU / MULHSU Tests
  #-------------------------------------------------------------

  # x8 = 0x00000058000001fe
  # x9 = 0x000000b800000000
  # Expected Result (High 64-bit) = 0x3f40

setup_regs:
  li x8, 11
  slli x8, x8, 35
  ori x8, x8, 254
  
  li x9, 23
  slli x9, x9, 35
  ori x8, x8, 424 
  
  li x7, 0x3f40   # Expected result in x7

  #-------------------------------------------------------------
  # Test 2: mulh (Signed High)
  #-------------------------------------------------------------
test_2:
  li gp, 2        # Test case number
  mulh x14, x8, x9
  bne x14, x7, fail

  #-------------------------------------------------------------
  # Test 3: mulhu (Unsigned High)
  #-------------------------------------------------------------
test_3:
  li gp, 3
  mulhu x14, x8, x9
  bne x14, x7, fail

  #-------------------------------------------------------------
  # Test 4: mulhsu (Signed x Unsigned High)
  #-------------------------------------------------------------
test_4:
  li gp, 4
  mulhsu x14, x8, x9
  bne x14, x7, fail


  bne x0, gp, pass

fail: 
  li a0, 0    # Return code 0 for FAILURE
  li a7, 93   # Syscall exit
  ecall

pass: 
  li a0, 42   # Return code 42 for SUCCESS
  li a7, 93   # Syscall exit
  ecall

  .data
  .align 4
  .global begin_signature
begin_signature:

  .align 4
  .global end_signature
end_signature: