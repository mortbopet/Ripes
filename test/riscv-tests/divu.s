.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li x1, 20
 li x2, 6
 divu x30, x1, x2
 li x29, 3
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, -20
 li x2, 6
 divu x30, x1, x2
 li x29, 715827879
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 20
 li x2, -6
 divu x30, x1, x2
 li x29, 0
 li gp, 4
 bne x30, x29, fail


  test_5:
 li x1, -20
 li x2, -6
 divu x30, x1, x2
 li x29, 0
 li gp, 5
 bne x30, x29, fail



  test_6:
 li x1, 0x80000000
 li x2, 1
 divu x30, x1, x2
 li x29, 0x80000000
 li gp, 6
 bne x30, x29, fail


  test_7:
 li x1, 0x80000000
 li x2, -1
 divu x30, x1, x2
 li x29, 0
 li gp, 7
 bne x30, x29, fail



  test_8:
 li x1, 0x80000000
 li x2, 0
 divu x30, x1, x2
 li x29, -1
 li gp, 8
 bne x30, x29, fail


  test_9:
 li x1, 1
 li x2, 0
 divu x30, x1, x2
 li x29, -1
 li gp, 9
 bne x30, x29, fail


  test_10:
 li x1, 0
 li x2, 0
 divu x30, x1, x2
 li x29, -1
 li gp, 10
 bne x30, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

