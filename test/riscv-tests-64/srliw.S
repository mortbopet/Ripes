.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li x1, 0xffffffff80000000
 srliw x14, x1, 0
 li x7, 0xffffffff80000000
 li gp, 2
 bne x14, x7, fail

  test_3: li x1, 0xffffffff80000000
 srliw x14, x1, 1
 li x7, 0x0000000040000000
 li gp, 3
 bne x14, x7, fail

  test_4: li x1, 0xffffffff80000000
 srliw x14, x1, 7
 li x7, 0x0000000001000000
 li gp, 4
 bne x14, x7, fail

  test_5: li x1, 0xffffffff80000000
 srliw x14, x1, 14
 li x7, 0x0000000000020000
 li gp, 5
 bne x14, x7, fail

  test_6: li x1, 0xffffffff80000001
 srliw x14, x1, 31
 li x7, 0x0000000000000001
 li gp, 6
 bne x14, x7, fail


  test_7: li x1, 0xffffffffffffffff
 srliw x14, x1, 0
 li x7, 0xffffffffffffffff
 li gp, 7
 bne x14, x7, fail

  test_8: li x1, 0xffffffffffffffff
 srliw x14, x1, 1
 li x7, 0x000000007fffffff
 li gp, 8
 bne x14, x7, fail

  test_9: li x1, 0xffffffffffffffff
 srliw x14, x1, 7
 li x7, 0x0000000001ffffff
 li gp, 9
 bne x14, x7, fail

  test_10: li x1, 0xffffffffffffffff
 srliw x14, x1, 14
 li x7, 0x000000000003ffff
 li gp, 10
 bne x14, x7, fail

  test_11: li x1, 0xffffffffffffffff
 srliw x14, x1, 31
 li x7, 0x0000000000000001
 li gp, 11
 bne x14, x7, fail


  test_12: li x1, 0x0000000021212121
 srliw x14, x1, 0
 li x7, 0x0000000021212121
 li gp, 12
 bne x14, x7, fail

  test_13: li x1, 0x0000000021212121
 srliw x14, x1, 1
 li x7, 0x0000000010909090
 li gp, 13
 bne x14, x7, fail

  test_14: li x1, 0x0000000021212121
 srliw x14, x1, 7
 li x7, 0x0000000000424242
 li gp, 14
 bne x14, x7, fail

  test_15: li x1, 0x0000000021212121
 srliw x14, x1, 14
 li x7, 0x0000000000008484
 li gp, 15
 bne x14, x7, fail

  test_16: li x1, 0x0000000021212121
 srliw x14, x1, 31
 li x7, 0x0000000000000000
 li gp, 16
 bne x14, x7, fail


  # Verify that shifts ignore top 32 (using true 64-bit values)

  test_44: li x1, 0xffffffff12345678
 srliw x14, x1, 0
 li x7, 0x0000000012345678
 li gp, 44
 bne x14, x7, fail

  test_45: li x1, 0xffffffff12345678
 srliw x14, x1, 4
 li x7, 0x0000000001234567
 li gp, 45
 bne x14, x7, fail

  test_46: li x1, 0x0000000092345678
 srliw x14, x1, 0
 li x7, 0xffffffff92345678
 li gp, 46
 bne x14, x7, fail

  test_47: li x1, 0x0000000092345678
 srliw x14, x1, 4
 li x7, 0x0000000009234567
 li gp, 47
 bne x14, x7, fail


  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_17: li x1, 0xffffffff80000000
 srliw x1, x1, 7
 li x7, 0x0000000001000000
 li gp, 17
 bne x1, x7, fail


  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_18: li x4, 0
 li x1, 0xffffffff80000000
 srliw x14, x1, 7
 addi x6, x14, 0
 li x7, 0x0000000001000000
 li gp, 18
 bne x6, x7, fail

  test_19: li x4, 0
 li x1, 0xffffffff80000000
 srliw x14, x1, 14
 nop
 addi x6, x14, 0
 li x7, 0x0000000000020000
 li gp, 19
 bne x6, x7, fail

  test_20: li x4, 0
 li x1, 0xffffffff80000001
 srliw x14, x1, 31
 nop
 nop
 addi x6, x14, 0
 li x7, 0x0000000000000001
 li gp, 20
 bne x6, x7, fail


  test_21: li x4, 0
 li x1, 0xffffffff80000000
 srliw x14, x1, 7
 li x7, 0x0000000001000000
 li gp, 21
 bne x14, x7, fail

  test_22: li x4, 0
 li x1, 0xffffffff80000000
 nop
 srliw x14, x1, 14
 li x7, 0x0000000000020000
 li gp, 22
 bne x14, x7, fail

  test_23: li x4, 0
 li x1, 0xffffffff80000001
 nop
 nop
 srliw x14, x1, 31
 li x7, 0x0000000000000001
 li gp, 23
 bne x14, x7, fail


  test_24: srliw x1, x0, 31
 li x7, 0
 li gp, 24
 bne x1, x7, fail

  test_25: li x1, 31
 srliw x0, x1, 28
 li x7, 0
 li gp, 25
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
