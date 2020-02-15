.text
main:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

test_2:
 li x1, 0x00000000
 addi x30, x1, 0x000
 li x29, 0x00000000
 li gp, 2
 bne x30, x29, fail

test_3:
 li x1, 0x00000001
 addi x30, x1, 0x001
 li x29, 0x00000002
 li gp, 3
 bne x30, x29, fail

test_4:
 li x1, 0x00000003
 addi x30, x1, 0x007
 li x29, 0x0000000a
 li gp, 4
 bne x30, x29, fail

test_5:
 li x1, 0x00000000
 addi x30, x1, 0xfffff800
 li x29, 0xfffff800
 li gp, 5
 bne x30, x29, fail

test_6:
 li x1, 0x80000000
 addi x30, x1, 0x000
 li x29, 0x80000000
 li gp, 6
 bne x30, x29, fail

test_7:
 li x1, 0x80000000
 addi x30, x1, 0xfffff800
 li x29, 0x7ffff800
 li gp, 7
 bne x30, x29, fail

test_8:
 li x1, 0x00000000
 addi x30, x1, 0x7ff
 li x29, 0x000007ff
 li gp, 8
 bne x30, x29, fail

test_9:
 li x1, 0x7fffffff
 addi x30, x1, 0x000
 li x29, 0x7fffffff
 li gp, 9
 bne x30, x29, fail

test_10:
 li x1, 0x7fffffff
 addi x30, x1, 0x7ff
 li x29, 0x800007fe
 li gp, 10
 bne x30, x29, fail

test_11:
 li x1, 0x80000000
 addi x30, x1, 0x7ff
 li x29, 0x800007ff
 li gp, 11
 bne x30, x29, fail

test_12:
 li x1, 0x7fffffff
 addi x30, x1, 0xfffff800
 li x29, 0x7ffff7ff
 li gp, 12
 bne x30, x29, fail

test_13:
 li x1, 0x00000000
 addi x30, x1, 0xffffffff
 li x29, 0xffffffff
 li gp, 13
 bne x30, x29, fail

test_14:
 li x1, 0xffffffff
 addi x30, x1, 0x001
 li x29, 0x00000000
 li gp, 14
 bne x30, x29, fail

test_15:
 li x1, 0xffffffff
 addi x30, x1, 0xffffffff
 li x29, 0xfffffffe
 li gp, 15
 bne x30, x29, fail

test_16: li x1, 0x7fffffff
 addi x30, x1, 0x001
 li x29, 0x80000000
 li gp, 16
 bne x30, x29, fail

  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

test_17:
 li x1, 13
 addi x1, x1, 11
 li x29, 24
 li gp, 17
 bne x1, x29, fail

test_24:
 addi x1, x0, 32
 li x29, 32
 li gp, 24
 bne x1, x29, fail

test_25:
 li x1, 33
 addi x0, x1, 50
 li x29, 0
 li gp, 25
 bne x0, x29, fail

pass:
 li a0, 42
 li a7, 93
 ecall

fail:
 li a0, 0
 li a7, 93
