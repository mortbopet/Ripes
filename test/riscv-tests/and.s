.text
main:
  #-------------------------------------------------------------
  # Logical tests
  #-------------------------------------------------------------

test_2:
 li x1, 0xff00ff00
 li x2, 0x0f0f0f0f
 and x30, x1, x2
 li x29, 0x0f000f00
 li gp, 2
 bne x30, x29, fail


test_3:
 li x1, 0x0ff00ff0
 li x2, 0xf0f0f0f0
 and x30, x1, x2
 li x29, 0x00f000f0
 li gp, 3
 bne x30, x29, fail


test_4:
 li x1, 0x00ff00ff
 li x2, 0x0f0f0f0f
 and x30, x1, x2
 li x29, 0x000f000f
 li gp, 4
 bne x30, x29, fail


test_5:
 li x1, 0xf00ff00f
 li x2, 0xf0f0f0f0
 and x30, x1, x2
 li x29, 0xf000f000
 li gp, 5
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

test_6:
 li x1, 0xff00ff00
 li x2, 0x0f0f0f0f
 and x1, x1, x2
 li x29, 0x0f000f00
 li gp, 6
 bne x1, x29, fail


test_7:
 li x1, 0x0ff00ff0
 li x2, 0xf0f0f0f0
 and x2, x1, x2
 li x29, 0x00f000f0
 li gp, 7
 bne x2, x29, fail


test_8:
 li x1, 0xff00ff00
 and x1, x1, x1
 li x29, 0xff00ff00
 li gp, 8
 bne x1, x29, fail



pass:
	li a0, 42
	li a7, 93
	ecall
fail:
	li a0, 0
	li a7, 93
	ecall
