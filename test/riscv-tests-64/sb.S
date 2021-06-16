.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  test_2: la x1, tdat
 li x2, 0xffffffffffffffaa
 sb x2, 0(x1)
 lb x14, 0(x1)
 li x7, 0xffffffffffffffaa
 li gp, 2
 bne x14, x7, fail

  test_3: la x1, tdat
 li x2, 0x0000000000000000
 sb x2, 1(x1)
 lb x14, 1(x1)
 li x7, 0x0000000000000000
 li gp, 3
 bne x14, x7, fail

  test_4: la x1, tdat
 li x2, 0xffffffffffffefa0
 sb x2, 2(x1)
 lh x14, 2(x1)
 li x7, 0xffffffffffffefa0
 li gp, 4
 bne x14, x7, fail

  test_5: la x1, tdat
 li x2, 0x000000000000000a
 sb x2, 3(x1)
 lb x14, 3(x1)
 li x7, 0x000000000000000a
 li gp, 5
 bne x14, x7, fail


  # Test with negative offset

  test_6: la x1, tdat8
 li x2, 0xffffffffffffffaa
 sb x2, -3(x1)
 lb x14, -3(x1)
 li x7, 0xffffffffffffffaa
 li gp, 6
 bne x14, x7, fail

  test_7: la x1, tdat8
 li x2, 0x0000000000000000
 sb x2, -2(x1)
 lb x14, -2(x1)
 li x7, 0x0000000000000000
 li gp, 7
 bne x14, x7, fail

  test_8: la x1, tdat8
 li x2, 0xffffffffffffffa0
 sb x2, -1(x1)
 lb x14, -1(x1)
 li x7, 0xffffffffffffffa0
 li gp, 8
 bne x14, x7, fail

  test_9: la x1, tdat8
 li x2, 0x000000000000000a
 sb x2, 0(x1)
 lb x14, 0(x1)
 li x7, 0x000000000000000a
 li gp, 9
 bne x14, x7, fail


  # Test with a negative base

  test_10: la x1, tdat9
 li x2, 0x12345678
 addi x4, x1, -32
 sb x2, 32(x4)
 lb x5, 0(x1)
 li x7, 0x78
 li gp, 10
 bne x5, x7, fail








  # Test with unaligned base

  test_11: la x1, tdat9
 li x2, 0x00003098
 addi x1, x1, -6
 sb x2, 7(x1)
 la x4, tdat10
 lb x5, 0(x4)
 li x7, 0xffffffffffffff98
 li gp, 11
 bne x5, x7, fail

# 53 "isa/rv64ui/sb.S"
  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_12: li gp, 12
 li x4, 0
 li x1, 0xffffffffffffffdd
 la x2, tdat
 sb x1, 0(x2)
 lb x14, 0(x2)
 li x7, 0xffffffffffffffdd
 bne x14, x7, fail

  test_13: li gp, 13
 li x4, 0
 li x1, 0xffffffffffffffcd
 la x2, tdat
 nop
 sb x1, 1(x2)
 lb x14, 1(x2)
 li x7, 0xffffffffffffffcd
 bne x14, x7, fail

  test_14: li gp, 14
 li x4, 0
 li x1, 0xffffffffffffffcc
 la x2, tdat
 nop
 nop
 sb x1, 2(x2)
 lb x14, 2(x2)
 li x7, 0xffffffffffffffcc
 bne x14, x7, fail

  test_15: li gp, 15
 li x4, 0
 li x1, 0xffffffffffffffbc
 nop
 la x2, tdat
 sb x1, 3(x2)
 lb x14, 3(x2)
 li x7, 0xffffffffffffffbc
 bne x14, x7, fail

  test_16: li gp, 16
 li x4, 0
 li x1, 0xffffffffffffffbb
 nop
 la x2, tdat
 nop
 sb x1, 4(x2)
 lb x14, 4(x2)
 li x7, 0xffffffffffffffbb
 bne x14, x7, fail

  test_17: li gp, 17
 li x4, 0
 li x1, 0xffffffffffffffab
 nop
 nop
 la x2, tdat
 sb x1, 5(x2)
 lb x14, 5(x2)
 li x7, 0xffffffffffffffab
 bne x14, x7, fail


  test_18: li gp, 18
 li x4, 0
 la x2, tdat
 li x1, 0x33
 sb x1, 0(x2)
 lb x14, 0(x2)
 li x7, 0x33
 bne x14, x7, fail

  test_19: li gp, 19
 li x4, 0
 la x2, tdat
 li x1, 0x23
 nop
 sb x1, 1(x2)
 lb x14, 1(x2)
 li x7, 0x23
 bne x14, x7, fail

  test_20: li gp, 20
 li x4, 0
 la x2, tdat
 li x1, 0x22
 nop
 nop
 sb x1, 2(x2)
 lb x14, 2(x2)
 li x7, 0x22
 bne x14, x7, fail

  test_21: li gp, 21
 li x4, 0
 la x2, tdat
 nop
 li x1, 0x12
 sb x1, 3(x2)
 lb x14, 3(x2)
 li x7, 0x12
 bne x14, x7, fail

  test_22: li gp, 22
 li x4, 0
 la x2, tdat
 nop
 li x1, 0x11
 nop
 sb x1, 4(x2)
 lb x14, 4(x2)
 li x7, 0x11
 bne x14, x7, fail

  test_23: li gp, 23
 li x4, 0
 la x2, tdat
 nop
 nop
 li x1, 0x01
 sb x1, 5(x2)
 lb x14, 5(x2)
 li x7, 0x01
 bne x14, x7, fail


  li a0, 0xef
  la a1, tdat
  sb a0, 3(a1)

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
tdat1: .byte 0xef
tdat2: .byte 0xef
tdat3: .byte 0xef
tdat4: .byte 0xef
tdat5: .byte 0xef
tdat6: .byte 0xef
tdat7: .byte 0xef
tdat8: .byte 0xef
tdat9: .byte 0xef
tdat10: .byte 0xef

.align 4
 .global end_signature
 end_signature:
