.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  test_2: la x1, tdat
 lb x14, 0(x1)
 li x7, 0xffffffffffffffff
 li gp, 2
 bne x14, x7, fail

  test_3: la x1, tdat
 lb x14, 1(x1)
 li x7, 0x0000000000000000
 li gp, 3
 bne x14, x7, fail

  test_4: la x1, tdat
 lb x14, 2(x1)
 li x7, 0xfffffffffffffff0
 li gp, 4
 bne x14, x7, fail

  test_5: la x1, tdat
 lb x14, 3(x1)
 li x7, 0x000000000000000f
 li gp, 5
 bne x14, x7, fail


  # Test with negative offset

  test_6: la x1, tdat4
 lb x14, -3(x1)
 li x7, 0xffffffffffffffff
 li gp, 6
 bne x14, x7, fail

  test_7: la x1, tdat4
 lb x14, -2(x1)
 li x7, 0x0000000000000000
 li gp, 7
 bne x14, x7, fail

  test_8: la x1, tdat4
 lb x14, -1(x1)
 li x7, 0xfffffffffffffff0
 li gp, 8
 bne x14, x7, fail

  test_9: la x1, tdat4
 lb x14, 0(x1)
 li x7, 0x000000000000000f
 li gp, 9
 bne x14, x7, fail


  # Test with a negative base

  test_10: la x1, tdat
 addi x1, x1, -32
 lb x5, 32(x1)
 li x7, 0xffffffffffffffff
 li gp, 10
 bne x5, x7, fail






  # Test with unaligned base

  test_11: la x1, tdat
 addi x1, x1, -6
 lb x5, 7(x1)
 li x7, 0x0000000000000000
 li gp, 11
 bne x5, x7, fail






  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_12: li gp, 12
 li x4, 0
 la x1, tdat2
 lb x14, 1(x1)
 addi x6, x14, 0
 li x7, 0xfffffffffffffff0
 bne x6, x7, fail

  test_13: li gp, 13
 li x4, 0
 la x1, tdat3
 lb x14, 1(x1)
 nop
 addi x6, x14, 0
 li x7, 0x000000000000000f
 bne x6, x7, fail

  test_14: li gp, 14
 li x4, 0
 la x1, tdat1
 lb x14, 1(x1)
 nop
 nop
 addi x6, x14, 0
 li x7, 0x0000000000000000
 bne x6, x7, fail


  test_15: li gp, 15
 li x4, 0
 la x1, tdat2
 lb x14, 1(x1)
 li x7, 0xfffffffffffffff0
 bne x14, x7, fail

  test_16: li gp, 16
 li x4, 0
 la x1, tdat3
 nop
 lb x14, 1(x1)
 li x7, 0x000000000000000f
 bne x14, x7, fail

  test_17: li gp, 17
 li x4, 0
 la x1, tdat1
 nop
 nop
 lb x14, 1(x1)
 li x7, 0x0000000000000000
 bne x14, x7, fail


  #-------------------------------------------------------------
  # Test write-after-write hazard
  #-------------------------------------------------------------

  test_18: la x5, tdat
 lb x2, 0(x5)
 li x2, 2
 li x7, 2
 li gp, 18
 bne x2, x7, fail






  test_19: la x5, tdat
 lb x2, 0(x5)
 nop
 li x2, 2
 li x7, 2
 li gp, 19
 bne x2, x7, fail







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

 

tdat:
tdat1: .byte 0xff
tdat2: .byte 0x00
tdat3: .byte 0xf0
tdat4: .byte 0x0f

.align 4
 .global end_signature
 end_signature:
