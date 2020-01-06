.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li x1, 0x00000000
 srai x30, x1, 0
 li x29, 0x00000000
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, 0x80000000
 srai x30, x1, 1
 li x29, 0xc0000000
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 0x80000000
 srai x30, x1, 7
 li x29, 0xff000000
 li gp, 4
 bne x30, x29, fail


  test_5:
 li x1, 0x80000000
 srai x30, x1, 14
 li x29, 0xfffe0000
 li gp, 5
 bne x30, x29, fail


  test_6:
 li x1, 0x80000001
 srai x30, x1, 31
 li x29, 0xffffffff
 li gp, 6
 bne x30, x29, fail



  test_7:
 li x1, 0x7fffffff
 srai x30, x1, 0
 li x29, 0x7fffffff
 li gp, 7
 bne x30, x29, fail


  test_8:
 li x1, 0x7fffffff
 srai x30, x1, 1
 li x29, 0x3fffffff
 li gp, 8
 bne x30, x29, fail


  test_9:
 li x1, 0x7fffffff
 srai x30, x1, 7
 li x29, 0x00ffffff
 li gp, 9
 bne x30, x29, fail


  test_10:
 li x1, 0x7fffffff
 srai x30, x1, 14
 li x29, 0x0001ffff
 li gp, 10
 bne x30, x29, fail


  test_11:
 li x1, 0x7fffffff
 srai x30, x1, 31
 li x29, 0x00000000
 li gp, 11
 bne x30, x29, fail



  test_12:
 li x1, 0x81818181
 srai x30, x1, 0
 li x29, 0x81818181
 li gp, 12
 bne x30, x29, fail


  test_13:
 li x1, 0x81818181
 srai x30, x1, 1
 li x29, 0xc0c0c0c0
 li gp, 13
 bne x30, x29, fail


  test_14:
 li x1, 0x81818181
 srai x30, x1, 7
 li x29, 0xff030303
 li gp, 14
 bne x30, x29, fail


  test_15:
 li x1, 0x81818181
 srai x30, x1, 14
 li x29, 0xfffe0606
 li gp, 15
 bne x30, x29, fail


  test_16:
 li x1, 0x81818181
 srai x30, x1, 31
 li x29, 0xffffffff
 li gp, 16
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_17:
 li x1, 0x80000000
 srai x1, x1, 7
 li x29, 0xff000000
 li gp, 17
 bne x1, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

