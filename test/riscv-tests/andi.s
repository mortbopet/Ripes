.text
main:
  #-------------------------------------------------------------
  # Logical tests
  #-------------------------------------------------------------

test_2:
 li x1, 0xff00ff00
 andi x30, x1, 0xffffff0f
 li x29, 0xff00ff00
 li gp, 2
 bne x30, x29, fail


test_3:
 li x1, 0x0ff00ff0
 andi x30, x1, 0x0f0
 li x29, 0x000000f0
 li gp, 3
 bne x30, x29, fail


test_4:
 li x1, 0x00ff00ff
 andi x30, x1, 0x70f
 li x29, 0x0000000f
 li gp, 4
 bne x30, x29, fail


test_5:
 li x1, 0xf00ff00f
 andi x30, x1, 0x0f0
 li x29, 0x00000000
 li gp, 5
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

test_6:
 li x1, 0xff00ff00
 andi x1, x1, 0x0f0
 li x29, 0x00000000
 li gp, 6
 bne x1, x29, fail

test_13:
 andi x1, x0, 0x0f0
 li x29, 0
 li gp, 13
 bne x1, x29, fail


test_14:
 li x1, 0x00ff00ff
 andi x0, x1, 0x70f
 li x29, 0
 li gp, 14
 bne x0, x29, fail

pass:
	li a0, 42
	li a7, 93
	ecall
fail:
	li a0, 0
	li a7, 93
	ecall
