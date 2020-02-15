.text
 main:


  #-------------------------------------------------------------
  # Logical tests
  #-------------------------------------------------------------

  test_2:
 li x1, 0x00ff0f00
 xori x30, x1, 0xffffff0f
 li x29, 0xff00f00f
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, 0x0ff00ff0
 xori x30, x1, 0x0f0
 li x29, 0x0ff00f00
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 0x00ff08ff
 xori x30, x1, 0x70f
 li x29, 0x00ff0ff0
 li gp, 4
 bne x30, x29, fail


  test_5:
 li x1, 0xf00ff00f
 xori x30, x1, 0x0f0
 li x29, 0xf00ff0ff
 li gp, 5
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_6:
 li x1, 0xff00f700
 xori x1, x1, 0x70f
 li x29, 0xff00f00f
 li gp, 6
 bne x1, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

