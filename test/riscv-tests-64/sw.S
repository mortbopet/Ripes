.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  test_2: la x1, tdat
 li x2, 0x0000000000aa00aa
 sw x2, 0(x1)
 lw x14, 0(x1)
 li x7, 0x0000000000aa00aa
 li gp, 2
 bne x14, x7, fail

  test_3: la x1, tdat
 li x2, 0xffffffffaa00aa00
 sw x2, 4(x1)
 lw x14, 4(x1)
 li x7, 0xffffffffaa00aa00
 li gp, 3
 bne x14, x7, fail

  test_4: la x1, tdat
 li x2, 0x000000000aa00aa0
 sw x2, 8(x1)
 lw x14, 8(x1)
 li x7, 0x000000000aa00aa0
 li gp, 4
 bne x14, x7, fail

  test_5: la x1, tdat
 li x2, 0xffffffffa00aa00a
 sw x2, 12(x1)
 lw x14, 12(x1)
 li x7, 0xffffffffa00aa00a
 li gp, 5
 bne x14, x7, fail


  # Test with negative offset

  test_6: la x1, tdat8
 li x2, 0x0000000000aa00aa
 sw x2, -12(x1)
 lw x14, -12(x1)
 li x7, 0x0000000000aa00aa
 li gp, 6
 bne x14, x7, fail

  test_7: la x1, tdat8
 li x2, 0xffffffffaa00aa00
 sw x2, -8(x1)
 lw x14, -8(x1)
 li x7, 0xffffffffaa00aa00
 li gp, 7
 bne x14, x7, fail

  test_8: la x1, tdat8
 li x2, 0x000000000aa00aa0
 sw x2, -4(x1)
 lw x14, -4(x1)
 li x7, 0x000000000aa00aa0
 li gp, 8
 bne x14, x7, fail

  test_9: la x1, tdat8
 li x2, 0xffffffffa00aa00a
 sw x2, 0(x1)
 lw x14, 0(x1)
 li x7, 0xffffffffa00aa00a
 li gp, 9
 bne x14, x7, fail


  # Test with a negative base

  test_10: la x1, tdat9
 li x2, 0x12345678
 addi x4, x1, -32
 sw x2, 32(x4)
 lw x5, 0(x1)
 li x7, 0x12345678
 li gp, 10
 bne x5, x7, fail








  # Test with unaligned base

  test_11: la x1, tdat9
 li x2, 0x58213098
 addi x1, x1, -3
 sw x2, 7(x1)
 la x4, tdat10
 lw x5, 0(x4)
 li x7, 0x58213098
 li gp, 11
 bne x5, x7, fail

# 53 "isa/rv64ui/sw.S"
  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_12: li gp, 12
 li x4, 0
 li x1, 0xffffffffaabbccdd
 la x2, tdat
 sw x1, 0(x2)
 lw x14, 0(x2)
 li x7, 0xffffffffaabbccdd
 bne x14, x7, fail

  test_13: li gp, 13
 li x4, 0
 li x1, 0xffffffffdaabbccd
 la x2, tdat
 nop
 sw x1, 4(x2)
 lw x14, 4(x2)
 li x7, 0xffffffffdaabbccd
 bne x14, x7, fail

  test_14: li gp, 14
 li x4, 0
 li x1, 0xffffffffddaabbcc
 la x2, tdat
 nop
 nop
 sw x1, 8(x2)
 lw x14, 8(x2)
 li x7, 0xffffffffddaabbcc
 bne x14, x7, fail

  test_15: li gp, 15
 li x4, 0
 li x1, 0xffffffffcddaabbc
 nop
 la x2, tdat
 sw x1, 12(x2)
 lw x14, 12(x2)
 li x7, 0xffffffffcddaabbc
 bne x14, x7, fail

  test_16: li gp, 16
 li x4, 0
 li x1, 0xffffffffccddaabb
 nop
 la x2, tdat
 nop
 sw x1, 16(x2)
 lw x14, 16(x2)
 li x7, 0xffffffffccddaabb
 bne x14, x7, fail

  test_17: li gp, 17
 li x4, 0
 li x1, 0xffffffffbccddaab
 nop
 nop
 la x2, tdat
 sw x1, 20(x2)
 lw x14, 20(x2)
 li x7, 0xffffffffbccddaab
 bne x14, x7, fail


  test_18: li gp, 18
 li x4, 0
 la x2, tdat
 li x1, 0x00112233
 sw x1, 0(x2)
 lw x14, 0(x2)
 li x7, 0x00112233
 bne x14, x7, fail

  test_19: li gp, 19
 li x4, 0
 la x2, tdat
 li x1, 0x30011223
 nop
 sw x1, 4(x2)
 lw x14, 4(x2)
 li x7, 0x30011223
 bne x14, x7, fail

  test_20: li gp, 20
 li x4, 0
 la x2, tdat
 li x1, 0x33001122
 nop
 nop
 sw x1, 8(x2)
 lw x14, 8(x2)
 li x7, 0x33001122
 bne x14, x7, fail

  test_21: li gp, 21
 li x4, 0
 la x2, tdat
 nop
 li x1, 0x23300112
 sw x1, 12(x2)
 lw x14, 12(x2)
 li x7, 0x23300112
 bne x14, x7, fail

  test_22: li gp, 22
 li x4, 0
 la x2, tdat
 nop
 li x1, 0x22330011
 nop
 sw x1, 16(x2)
 lw x14, 16(x2)
 li x7, 0x22330011
 bne x14, x7, fail

  test_23: li gp, 23
 li x4, 0
 la x2, tdat
 nop
 nop
 li x1, 0x12233001
 sw x1, 20(x2)
 lw x14, 20(x2)
 li x7, 0x12233001
 bne x14, x7, fail


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
tdat1: .word 0xdeadbeef
tdat2: .word 0xdeadbeef
tdat3: .word 0xdeadbeef
tdat4: .word 0xdeadbeef
tdat5: .word 0xdeadbeef
tdat6: .word 0xdeadbeef
tdat7: .word 0xdeadbeef
tdat8: .word 0xdeadbeef
tdat9: .word 0xdeadbeef
tdat10: .word 0xdeadbeef

.align 4
 .global end_signature
 end_signature:
