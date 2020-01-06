.text
main:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

test_2:
 li x1, 0x00000001
 slli x30, x1, 0
 li x29, 0x00000001
 li gp, 2
 bne x30, x29, fail


test_3:
 li x1, 0x00000001
 slli x30, x1, 1
 li x29, 0x00000002
 li gp, 3
 bne x30, x29, fail


test_4:
 li x1, 0x00000001
 slli x30, x1, 7
 li x29, 0x00000080
 li gp, 4
 bne x30, x29, fail


test_5:
 li x1, 0x00000001
 slli x30, x1, 14
 li x29, 0x00004000
 li gp, 5
 bne x30, x29, fail


test_6:
 li x1, 0x00000001
 slli x30, x1, 31
 li x29, 0x80000000
 li gp, 6
 bne x30, x29, fail



test_7:
 li x1, 0xffffffff
 slli x30, x1, 0
 li x29, 0xffffffff
 li gp, 7
 bne x30, x29, fail


test_8:
 li x1, 0xffffffff
 slli x30, x1, 1
 li x29, 0xfffffffe
 li gp, 8
 bne x30, x29, fail


test_9:
 li x1, 0xffffffff
 slli x30, x1, 7
 li x29, 0xffffff80
 li gp, 9
 bne x30, x29, fail


test_10:
 li x1, 0xffffffff
 slli x30, x1, 14
 li x29, 0xffffc000
 li gp, 10
 bne x30, x29, fail


test_11:
 li x1, 0xffffffff
 slli x30, x1, 31
 li x29, 0x80000000
 li gp, 11
 bne x30, x29, fail



test_12:
 li x1, 0x21212121
 slli x30, x1, 0
 li x29, 0x21212121
 li gp, 12
 bne x30, x29, fail


test_13:
 li x1, 0x21212121
 slli x30, x1, 1
 li x29, 0x42424242
 li gp, 13
 bne x30, x29, fail


test_14:
 li x1, 0x21212121
 slli x30, x1, 7
 li x29, 0x90909080
 li gp, 14
 bne x30, x29, fail


test_15:
 li x1, 0x21212121
 slli x30, x1, 14
 li x29, 0x48484000
 li gp, 15
 bne x30, x29, fail


test_16:
 li x1, 0x21212121
 slli x30, x1, 31
 li x29, 0x80000000
 li gp, 16
 bne x30, x29, fail


 
  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

test_17:
 li x1, 0x00000001
 slli x1, x1, 7
 li x29, 0x00000080
 li gp, 17
 bne x1, x29, fail

pass:
	li a0, 42
	li a7, 93
	ecall

fail:
	li a0, 0
	li a7, 93
	ecall
