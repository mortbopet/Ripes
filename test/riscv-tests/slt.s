.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li x1, 0x00000000
 li x2, 0x00000000
 slt x30, x1, x2
 li x29, 0
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, 0x00000001
 li x2, 0x00000001
 slt x30, x1, x2
 li x29, 0
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 0x00000003
 li x2, 0x00000007
 slt x30, x1, x2
 li x29, 1
 li gp, 4
 bne x30, x29, fail


  test_5:
 li x1, 0x00000007
 li x2, 0x00000003
 slt x30, x1, x2
 li x29, 0
 li gp, 5
 bne x30, x29, fail



  test_6:
 li x1, 0x00000000
 li x2, 0xffff8000
 slt x30, x1, x2
 li x29, 0
 li gp, 6
 bne x30, x29, fail


  test_7:
 li x1, 0x80000000
 li x2, 0x00000000
 slt x30, x1, x2
 li x29, 1
 li gp, 7
 bne x30, x29, fail


  test_8:
 li x1, 0x80000000
 li x2, 0xffff8000
 slt x30, x1, x2
 li x29, 1
 li gp, 8
 bne x30, x29, fail



  test_9:
 li x1, 0x00000000
 li x2, 0x00007fff
 slt x30, x1, x2
 li x29, 1
 li gp, 9
 bne x30, x29, fail


  test_10:
 li x1, 0x7fffffff
 li x2, 0x00000000
 slt x30, x1, x2
 li x29, 0
 li gp, 10
 bne x30, x29, fail


  test_11:
 li x1, 0x7fffffff
 li x2, 0x00007fff
 slt x30, x1, x2
 li x29, 0
 li gp, 11
 bne x30, x29, fail



  test_12:
 li x1, 0x80000000
 li x2, 0x00007fff
 slt x30, x1, x2
 li x29, 1
 li gp, 12
 bne x30, x29, fail


  test_13:
 li x1, 0x7fffffff
 li x2, 0xffff8000
 slt x30, x1, x2
 li x29, 0
 li gp, 13
 bne x30, x29, fail



  test_14:
 li x1, 0x00000000
 li x2, 0xffffffff
 slt x30, x1, x2
 li x29, 0
 li gp, 14
 bne x30, x29, fail


  test_15:
 li x1, 0xffffffff
 li x2, 0x00000001
 slt x30, x1, x2
 li x29, 1
 li gp, 15
 bne x30, x29, fail


  test_16:
 li x1, 0xffffffff
 li x2, 0xffffffff
 slt x30, x1, x2
 li x29, 0
 li gp, 16
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_17:
 li x1, 14
 li x2, 13
 slt x1, x1, x2
 li x29, 0
 li gp, 17
 bne x1, x29, fail


  test_18:
 li x1, 11
 li x2, 13
 slt x2, x1, x2
 li x29, 1
 li gp, 18
 bne x2, x29, fail


  test_19:
 li x1, 13
 slt x1, x1, x1
 li x29, 0
 li gp, 19
 bne x1, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

