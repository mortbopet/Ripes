.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Logical tests
  #-------------------------------------------------------------

  test_2: li x1, 0xffffffffff00ff00
 ori x14, x1, 0xffffff0f
 li x7, 0xffffffffffffff0f
 li gp, 2
 bne x14, x7, fail

  test_3: li x1, 0x000000000ff00ff0
 ori x14, x1, 0x0f0
 li x7, 0x000000000ff00ff0
 li gp, 3
 bne x14, x7, fail

  test_4: li x1, 0x0000000000ff00ff
 ori x14, x1, 0x70f
 li x7, 0x0000000000ff07ff
 li gp, 4
 bne x14, x7, fail

  test_5: li x1, 0xfffffffff00ff00f
 ori x14, x1, 0x0f0
 li x7, 0xfffffffff00ff0ff
 li gp, 5
 bne x14, x7, fail


  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_6: li x1, 0xff00ff00
 ori x1, x1, 0x0f0
 li x7, 0xff00fff0
 li gp, 6
 bne x1, x7, fail


  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_7: li x4, 0
 li x1, 0x000000000ff00ff0
 ori x14, x1, 0x0f0
 addi x6, x14, 0
 li x7, 0x000000000ff00ff0
 li gp, 7
 bne x6, x7, fail

  test_8: li x4, 0
 li x1, 0x0000000000ff00ff
 ori x14, x1, 0x70f
 nop
 addi x6, x14, 0
 li x7, 0x0000000000ff07ff
 li gp, 8
 bne x6, x7, fail

  test_9: li x4, 0
 li x1, 0xfffffffff00ff00f
 ori x14, x1, 0x0f0
 nop
 nop
 addi x6, x14, 0
 li x7, 0xfffffffff00ff0ff
 li gp, 9
 bne x6, x7, fail


  test_10: li x4, 0
 li x1, 0x000000000ff00ff0
 ori x14, x1, 0x0f0
 li x7, 0x000000000ff00ff0
 li gp, 10
 bne x14, x7, fail

  test_11: li x4, 0
 li x1, 0x0000000000ff00ff
 nop
 ori x14, x1, 0xffffff0f
 li x7, 0xffffffffffffffff
 li gp, 11
 bne x14, x7, fail

  test_12: li x4, 0
 li x1, 0xfffffffff00ff00f
 nop
 nop
 ori x14, x1, 0x0f0
 li x7, 0xfffffffff00ff0ff
 li gp, 12
 bne x14, x7, fail


  test_13: ori x1, x0, 0x0f0
 li x7, 0x0f0
 li gp, 13
 bne x1, x7, fail

  test_14: li x1, 0x00ff00ff
 ori x0, x1, 0x70f
 li x7, 0
 li gp, 14
 bne x0, x7, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall
 pass: li a0, 42
 li a7, 93
 ecall



  .data
 .data 
 .align 4
 .global begin_signature
 begin_signature:

 

.align 4
 .global end_signature
 end_signature:
