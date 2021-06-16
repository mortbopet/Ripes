.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------




  test_2: li x1, 0xffffffff80000000
 li x2, 0
 srl x14, x1, x2
 li x7, 0xffffffff80000000
 li gp, 2
 bne x14, x7, fail

  test_3: li x1, 0xffffffff80000000
 li x2, 1
 srl x14, x1, x2
 li x7, 0x7fffffffC0000000
 li gp, 3
 bne x14, x7, fail

  test_4: li x1, 0xffffffff80000000
 li x2, 7
 srl x14, x1, x2
 li x7, 0x01fffffffF000000
 li gp, 4
 bne x14, x7, fail

  test_5: li x1, 0xffffffff80000000
 li x2, 14
 srl x14, x1, x2
 li x7, 0x0003fffffffE0000
 li gp, 5
 bne x14, x7, fail

  test_6: li x1, 0xffffffff80000001
 li x2, 31
 srl x14, x1, x2
 li x7, 0x00000001ffffffff
 li gp, 6
 bne x14, x7, fail


  test_7: li x1, 0xffffffffffffffff
 li x2, 0
 srl x14, x1, x2
 li x7, 0xffffffffffffffff
 li gp, 7
 bne x14, x7, fail

  test_8: li x1, 0xffffffffffffffff
 li x2, 1
 srl x14, x1, x2
 li x7, 0x7fffffffffffffff
 li gp, 8
 bne x14, x7, fail

  test_9: li x1, 0xffffffffffffffff
 li x2, 7
 srl x14, x1, x2
 li x7, 0x01ffffffffffffff
 li gp, 9
 bne x14, x7, fail

  test_10: li x1, 0xffffffffffffffff
 li x2, 14
 srl x14, x1, x2
 li x7, 0x0003ffffffffffff
 li gp, 10
 bne x14, x7, fail

  test_11: li x1, 0xffffffffffffffff
 li x2, 31
 srl x14, x1, x2
 li x7, 0x00000001ffffffff
 li gp, 11
 bne x14, x7, fail


  test_12: li x1, 0x0000000021212121
 li x2, 0
 srl x14, x1, x2
 li x7, 0x0000000021212121
 li gp, 12
 bne x14, x7, fail

  test_13: li x1, 0x0000000021212121
 li x2, 1
 srl x14, x1, x2
 li x7, 0x0000000010909090
 li gp, 13
 bne x14, x7, fail

  test_14: li x1, 0x0000000021212121
 li x2, 7
 srl x14, x1, x2
 li x7, 0x0000000000424242
 li gp, 14
 bne x14, x7, fail

  test_15: li x1, 0x0000000021212121
 li x2, 14
 srl x14, x1, x2
 li x7, 0x0000000000008484
 li gp, 15
 bne x14, x7, fail

  test_16: li x1, 0x0000000021212121
 li x2, 31
 srl x14, x1, x2
 li x7, 0
 li gp, 16
 bne x14, x7, fail


  # Verify that shifts only use bottom six(rv64) or five(rv32) bits

  test_17: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc0
 srl x14, x1, x2
 li x7, 0x0000000021212121
 li gp, 17
 bne x14, x7, fail

  test_18: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc1
 srl x14, x1, x2
 li x7, 0x0000000010909090
 li gp, 18
 bne x14, x7, fail

  test_19: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffc7
 srl x14, x1, x2
 li x7, 0x0000000000424242
 li gp, 19
 bne x14, x7, fail

  test_20: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffce
 srl x14, x1, x2
 li x7, 0x0000000000008484
 li gp, 20
 bne x14, x7, fail

  test_21: li x1, 0x0000000021212121
 li x2, 0xffffffffffffffff
 srl x14, x1, x2
 li x7, 0x0000000000000000
 li gp, 21
 bne x14, x7, fail


  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

# TODO: parser accepts this as sign extended value whereas gcc does not
# I modified it by adding a 0 in front; this applies to all 9 character constants 
  test_22: li x1, 0x080000000
 li x2, 7
 srl x1, x1, x2
 li x7, 0x01000000
 li gp, 22
 bne x1, x7, fail

  test_23: li x1, 0x080000000
 li x2, 14
 srl x2, x1, x2
 li x7, 0x00020000
 li gp, 23
 bne x2, x7, fail

  test_24: li x1, 7
 srl x1, x1, x1
 li x7, 0
 li gp, 24
 bne x1, x7, fail


  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_25: li x4, 0
 li x1, 0x080000000
 li x2, 7
 srl x14, x1, x2
 addi x6, x14, 0
 li x7, 0x01000000
 li gp, 25
 bne x6, x7, fail

  test_26: li x4, 0
 li x1, 0x080000000
 li x2, 14
 srl x14, x1, x2
 nop
 addi x6, x14, 0
 li x7, 0x00020000
 li gp, 26
 bne x6, x7, fail

  test_27: li x4, 0
 li x1, 0x080000000
 li x2, 31
 srl x14, x1, x2
 nop
 nop
 addi x6, x14, 0
 li x7, 0x00000001
 li gp, 27
 bne x6, x7, fail


  test_28: li x4, 0
 li x1, 0x080000000
 li x2, 7
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x01000000
 li gp, 28
 bne x14, x7, fail

  test_29: li x4, 0
 li x1, 0x080000000
 li x2, 14
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00020000
 li gp, 29
 bne x14, x7, fail

  test_30: li x4, 0
 li x1, 0x080000000
 li x2, 31
 nop
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00000001
 li gp, 30
 bne x14, x7, fail

  test_31: li x4, 0
 li x1, 0x080000000
 nop
 li x2, 7
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x01000000
 li gp, 31
 bne x14, x7, fail

  test_32: li x4, 0
 li x1, 0x080000000
 nop
 li x2, 14
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00020000
 li gp, 32
 bne x14, x7, fail

  test_33: li x4, 0
 li x1, 0x080000000
 nop
 nop
 li x2, 31
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00000001
 li gp, 33
 bne x14, x7, fail


  test_34: li x4, 0
 li x2, 7
 li x1, 0x080000000
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x01000000
 li gp, 34
 bne x14, x7, fail

  test_35: li x4, 0
 li x2, 14
 li x1, 0x080000000
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00020000
 li gp, 35
 bne x14, x7, fail

  test_36: li x4, 0
 li x2, 31
 li x1, 0x080000000
 nop
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00000001
 li gp, 36
 bne x14, x7, fail

  test_37: li x4, 0
 li x2, 7
 nop
 li x1, 0x080000000
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x01000000
 li gp, 37
 bne x14, x7, fail

  test_38: li x4, 0
 li x2, 14
 nop
 li x1, 0x080000000
 nop
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00020000
 li gp, 38
 bne x14, x7, fail

  test_39: li x4, 0
 li x2, 31
 nop
 nop
 li x1, 0x080000000
 srl x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 0x00000001
 li gp, 39
 bne x14, x7, fail


  test_40: li x1, 15
 srl x2, x0, x1
 li x7, 0
 li gp, 40
 bne x2, x7, fail

  test_41: li x1, 32
 srl x2, x1, x0
 li x7, 32
 li gp, 41
 bne x2, x7, fail

  test_42: srl x1, x0, x0
 li x7, 0
 li gp, 42
 bne x1, x7, fail

  test_43: li x1, 1024
 li x2, 2048
 srl x0, x1, x2
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
