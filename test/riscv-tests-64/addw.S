.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li x1, 0x00000000
 li x2, 0x00000000
 addw x14, x1, x2
 li x7, 0x00000000
 li gp, 2
 bne x14, x7, fail

  test_3: li x1, 0x00000001
 li x2, 0x00000001
 addw x14, x1, x2
 li x7, 0x00000002
 li gp, 3
 bne x14, x7, fail

  test_4: li x1, 0x00000003
 li x2, 0x00000007
 addw x14, x1, x2
 li x7, 0x0000000a
 li gp, 4
 bne x14, x7, fail


  test_5: li x1, 0x0000000000000000
 li x2, 0xffffffffffff8000
 addw x14, x1, x2
 li x7, 0xffffffffffff8000
 li gp, 5
 bne x14, x7, fail

  test_6: li x1, 0xffffffff80000000
 li x2, 0x00000000
 addw x14, x1, x2
 li x7, 0xffffffff80000000
 li gp, 6
 bne x14, x7, fail

  test_7: li x1, 0xffffffff80000000
 li x2, 0xffffffffffff8000
 addw x14, x1, x2
 li x7, 0x000000007fff8000
 li gp, 7
 bne x14, x7, fail


  test_8: li x1, 0x0000000000000000
 li x2, 0x0000000000007fff
 addw x14, x1, x2
 li x7, 0x0000000000007fff
 li gp, 8
 bne x14, x7, fail

  test_9: li x1, 0x000000007fffffff
 li x2, 0x0000000000000000
 addw x14, x1, x2
 li x7, 0x000000007fffffff
 li gp, 9
 bne x14, x7, fail

  test_10: li x1, 0x000000007fffffff
 li x2, 0x0000000000007fff
 addw x14, x1, x2
 li x7, 0xffffffff80007ffe
 li gp, 10
 bne x14, x7, fail


  test_11: li x1, 0xffffffff80000000
 li x2, 0x0000000000007fff
 addw x14, x1, x2
 li x7, 0xffffffff80007fff
 li gp, 11
 bne x14, x7, fail

  test_12: li x1, 0x000000007fffffff
 li x2, 0xffffffffffff8000
 addw x14, x1, x2
 li x7, 0x000000007fff7fff
 li gp, 12
 bne x14, x7, fail


  test_13: li x1, 0x0000000000000000
 li x2, 0xffffffffffffffff
 addw x14, x1, x2
 li x7, 0xffffffffffffffff
 li gp, 13
 bne x14, x7, fail

  test_14: li x1, 0xffffffffffffffff
 li x2, 0x0000000000000001
 addw x14, x1, x2
 li x7, 0x0000000000000000
 li gp, 14
 bne x14, x7, fail

  test_15: li x1, 0xffffffffffffffff
 li x2, 0xffffffffffffffff
 addw x14, x1, x2
 li x7, 0xfffffffffffffffe
 li gp, 15
 bne x14, x7, fail


  test_16: li x1, 0x0000000000000001
 li x2, 0x000000007fffffff
 addw x14, x1, x2
 li x7, 0xffffffff80000000
 li gp, 16
 bne x14, x7, fail


  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_17: li x1, 13
 li x2, 11
 addw x1, x1, x2
 li x7, 24
 li gp, 17
 bne x1, x7, fail

  test_18: li x1, 14
 li x2, 11
 addw x2, x1, x2
 li x7, 25
 li gp, 18
 bne x2, x7, fail

  test_19: li x1, 13
 addw x1, x1, x1
 li x7, 26
 li gp, 19
 bne x1, x7, fail


  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_20: li x4, 0
 li x1, 13
 li x2, 11
 addw x14, x1, x2
 addi x6, x14, 0
 li x7, 24
 li gp, 20
 bne x6, x7, fail

  test_21: li x4, 0
 li x1, 14
 li x2, 11
 addw x14, x1, x2
 nop
 addi x6, x14, 0
 li x7, 25
 li gp, 21
 bne x6, x7, fail

  test_22: li x4, 0
 li x1, 15
 li x2, 11
 addw x14, x1, x2
 nop
 nop
 addi x6, x14, 0
 li x7, 26
 li gp, 22
 bne x6, x7, fail


  test_23: li x4, 0
 li x1, 13
 li x2, 11
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 24
 li gp, 23
 bne x14, x7, fail

  test_24: li x4, 0
 li x1, 14
 li x2, 11
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 25
 li gp, 24
 bne x14, x7, fail

  test_25: li x4, 0
 li x1, 15
 li x2, 11
 nop
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 26
 li gp, 25
 bne x14, x7, fail

  test_26: li x4, 0
 li x1, 13
 nop
 li x2, 11
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 24
 li gp, 26
 bne x14, x7, fail

  test_27: li x4, 0
 li x1, 14
 nop
 li x2, 11
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 25
 li gp, 27
 bne x14, x7, fail

  test_28: li x4, 0
 li x1, 15
 nop
 nop
 li x2, 11
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 26
 li gp, 28
 bne x14, x7, fail


  test_29: li x4, 0
 li x2, 11
 li x1, 13
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 24
 li gp, 29
 bne x14, x7, fail

  test_30: li x4, 0
 li x2, 11
 li x1, 14
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 25
 li gp, 30
 bne x14, x7, fail

  test_31: li x4, 0
 li x2, 11
 li x1, 15
 nop
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 26
 li gp, 31
 bne x14, x7, fail

  test_32: li x4, 0
 li x2, 11
 nop
 li x1, 13
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 24
 li gp, 32
 bne x14, x7, fail

  test_33: li x4, 0
 li x2, 11
 nop
 li x1, 14
 nop
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 25
 li gp, 33
 bne x14, x7, fail

  test_34: li x4, 0
 li x2, 11
 nop
 nop
 li x1, 15
 addw x14, x1, x2
 addi x4, x4, 1
 li x5, 2
 li x7, 26
 li gp, 34
 bne x14, x7, fail


  test_35: li x1, 15
 addw x2, x0, x1
 li x7, 15
 li gp, 35
 bne x2, x7, fail

  test_36: li x1, 32
 addw x2, x1, x0
 li x7, 32
 li gp, 36
 bne x2, x7, fail

  test_37: addw x1, x0, x0
 li x7, 0
 li gp, 37
 bne x1, x7, fail

  test_38: li x1, 16
 li x2, 30
 addw x0, x1, x2
 li x7, 0
 li gp, 38
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
