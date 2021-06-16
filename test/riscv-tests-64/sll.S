.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li x1, 0x0000000000000001
 li x2, 0
 sll x14, x1, x2
 li x7, 0x0000000000000001
 li gp, 2
 bne x14, x7, fail

  test_3: li x1, 0x0000000000000001
 li x2, 1
 sll x14, x1, x2
 li x7, 0x0000000000000002
 li gp, 3
 bne x14, x7, fail

  test_4: li x1, 0x0000000000000001
 li x2, 7
 sll x14, x1, x2
 li x7, 0x0000000000000080
 li gp, 4
 bne x14, x7, fail

  test_5: li x1, 0x0000000000000001
 li x2, 14
 sll x14, x1, x2
 li x7, 0x0000000000004000
 li gp, 5
 bne x14, x7, fail

  test_6: li x1, 0x0000000000000001
 li x2, 31
 sll x14, x1, x2
 li x7, 0x0000000080000000
 li gp, 6
 bne x14, x7, fail


  test_7: li x1, 0xffffffffffffffff
 li x2, 0
 sll x14, x1, x2
 li x7, 0xffffffffffffffff
 li gp, 7
 bne x14, x7, fail

  test_8: li x1, 0xffffffffffffffff
 li x2, 1
 sll x14, x1, x2
 li x7, 0xfffffffffffffffe
 li gp, 8
 bne x14, x7, fail

  test_9: li x1, 0xffffffffffffffff
 li x2, 7
 sll x14, x1, x2
 li x7, 0xffffffffffffff80
 li gp, 9
 bne x14, x7, fail

  test_10: li x1, 0xffffffffffffffff
 li x2, 14
 sll x14, x1, x2
 li x7, 0xffffffffffffc000
 li gp, 10
 bne x14, x7, fail

  test_11: li x1, 0xffffffffffffffff
 li x2, 31
 sll x14, x1, x2
 li x7, 0xffffffff80000000
 li gp, 11
 bne x14, x7, fail


  test_12: li x1, 0x0000000021212121
 li x2, 0
 sll x14, x1, x2
 li x7, 0x0000000021212121
 li gp, 12
 bne x14, x7, fail

  test_13: li x1, 0x0000000021212121
 li x2, 1
 sll x14, x1, x2
 li x7, 0x0000000042424242
 li gp, 13
 bne x14, x7, fail

  test_14: li x1, 0x0000000021212121
 li x2, 7
 sll x14, x1, x2
 li x7, 0x0000001090909080
 li gp, 14
 bne x14, x7, fail

  test_15: li x1, 0x0000000021212121
 li x2, 14
 sll x14, x1, x2
 li x7, 0x0000084848484000
 li gp, 15
 bne x14, x7, fail

  test_16: li x1, 0x0000000021212121
 li x2, 31
 sll x14, x1, x2
 li x7, 0x1090909080000000
 li gp, 16
 bne x14, x7, fail


  # Verify that shifts only use bottom six(rv64) or five(rv32) bits

  test_17: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc0
 sll x14, x1, x2
 li x7, 0x0000000021212121
 li gp, 17
 bne x14, x7, fail

  test_18: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc1
 sll x14, x1, x2
 li x7, 0x0000000042424242
 li gp, 18
 bne x14, x7, fail

  test_19: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc7
 sll x14, x1, x2
 li x7, 0x0000001090909080
 li gp, 19
 bne x14, x7, fail

  test_20: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffce
 sll x14, x1, x2
 li x7, 0x0000084848484000
 li gp, 20
 bne x14, x7, fail



  test_21: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffff
 sll x14, x1, x2
 li x7, 0x8000000000000000
 li gp, 21
 bne x14, x7, fail

  test_50: li x1, 0x0000000000000001
 li x2, 63
 sll x14, x1, x2
 li x7, 0x8000000000000000
 li gp, 50
 bne x14, x7, fail

  test_51: li x1, 0xffffffffffffffff
 li x2, 39
 sll x14, x1, x2
 li x7, 0xffffff8000000000
 li gp, 51
 bne x14, x7, fail

  test_52: li x1, 0x0000000021212121
 li x2, 43
 sll x14, x1, x2
 li x7, 0x0909080000000000
 li gp, 52
 bne x14, x7, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_22: li x1, 0x00000001
 li x2, 7
 sll x1, x1, x2
 li x7, 0x00000080
 li gp, 22
 bne x1, x7, fail

  test_23: li x1, 0x00000001
 li x2, 14
 sll x2, x1, x2
 li x7, 0x00004000
 li gp, 23
 bne x2, x7, fail

  test_24: li x1, 3
 sll x1, x1, x1
 li x7, 24
 li gp, 24
 bne x1, x7, fail


  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_25: li x4, 0
 li x1, 0x0000000000000001
 li x2, 7
 sll x14, x1, x2
 addi x6, x14, 0
 li x7, 0x0000000000000080
 li gp, 25
 bne x6, x7, fail

  test_26: li x4, 0
 li x1, 0x0000000000000001
 li x2, 14
 sll x14, x1, x2
 nop
 addi x6, x14, 0
 li x7, 0x0000000000004000
 li gp, 26
 bne x6, x7, fail

  test_27: li x4, 0
 li x1, 0x0000000000000001
 li x2, 31
 sll x14, x1, x2
 nop
 nop
 addi x6, x14, 0
 li x7, 0x0000000080000000
 li gp, 27
 bne x6, x7, fail


  test_28: li x4, 0
 li x1, 0x0000000000000001
 li x2, 7
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000000080
 li gp, 28
 bne x14, x7, fail

  test_29: li x4, 0
 li x1, 0x0000000000000001
 li x2, 14
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000004000
 li gp, 29
 bne x14, x7, fail

  test_30: li x4, 0
 li x1, 0x0000000000000001
 li x2, 31
 nop
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000080000000
 li gp, 30
 bne x14, x7, fail

  test_31: li x4, 0
 li x1, 0x0000000000000001
 nop
 li x2, 7
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000000080
 li gp, 31
 bne x14, x7, fail

  test_32: li x4, 0
 li x1, 0x0000000000000001
 nop
 li x2, 14
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000004000
 li gp, 32
 bne x14, x7, fail

  test_33: li x4, 0
 li x1, 0x0000000000000001
 nop
 nop
 li x2, 31
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000080000000
 li gp, 33
 bne x14, x7, fail


  test_34: li x4, 0
 li x2, 7
 li x1, 0x0000000000000001
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000000080
 li gp, 34
 bne x14, x7, fail

  test_35: li x4, 0
 li x2, 14
 li x1, 0x0000000000000001
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000004000
 li gp, 35
 bne x14, x7, fail

  test_36: li x4, 0
 li x2, 31
 li x1, 0x0000000000000001
 nop
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000080000000
 li gp, 36
 bne x14, x7, fail

  test_37: li x4, 0
 li x2, 7
 nop
 li x1, 0x0000000000000001
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000000080
 li gp, 37
 bne x14, x7, fail

  test_38: li x4, 0
 li x2, 14
 nop
 li x1, 0x0000000000000001
 nop
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000000004000
 li gp, 38
 bne x14, x7, fail

  test_39: li x4, 0
 li x2, 31
 nop
 nop
 li x1, 0x0000000000000001
 sll x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x0000000080000000
 li gp, 39
 bne x14, x7, fail


  test_40: li x1, 15
 sll x2, x0, x1
 li x7, 0
 li gp, 40
 bne x2, x7, fail

  test_41: li x1, 32
 sll x2, x1, x0
 li x7, 32
 li gp, 41
 bne x2, x7, fail

  test_42: sll x1, x0, x0
 li x7, 0
 li gp, 42
 bne x1, x7, fail

  test_43: li x1, 1024
 li x2, 2048
 sll x0, x1, x2
 li x7, 0
 li gp, 43
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
